#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/select.h>
#include <sys/wait.h>

#include "../includes/master_functions.h"
#include "../includes/queries_master.h"
#include "../includes/message_handlers.h"
#include "../includes/util.h"

#include "../includes/patient_list.h"
#include "../includes/avl_tree.h"
#include "../includes/ht_avlt.h"



extern int errno;


void handler(int sig, siginfo_t *info, void *ucontext);


int queries_started = 0;
int exit_status = 0;
// set global variables so that all handlers have access to them
// flags for SIGINT SIGQUIT and queries
// volotile signal flags
volatile sig_atomic_t signal_sent_from_pid = 0;
volatile sig_atomic_t signal_occured = 0;
volatile sig_atomic_t signal_status = 0;
volatile sig_atomic_t signal_buffer[READ_BUFFER_SIZE] = {0};
volatile sig_atomic_t signal_sent_from_pid_buffer[READ_BUFFER_SIZE] = {0};


// handler for all the signals
void handler(int sig, siginfo_t *info, void *ucontext){

    signal_occured = 1;
    // add to signal queue
    add_signal(signal_buffer, signal_sent_from_pid_buffer, sig, info->si_pid);
}


int main(int argc, char** argv){

    sigset_t fullmask, int_quit_signal_mask, int_quit_chld_usr_signal_mask;
    sigfillset(&fullmask);
    // reset any blockd signal 
    sigprocmask(SIG_UNBLOCK, &fullmask, NULL);

    // create cusstom mask for this application
    sigemptyset(&int_quit_chld_usr_signal_mask);
    sigaddset(&int_quit_chld_usr_signal_mask, SIGINT);
    sigaddset(&int_quit_chld_usr_signal_mask, SIGQUIT);
    sigaddset(&int_quit_chld_usr_signal_mask, SIGCHLD);
    sigaddset(&int_quit_chld_usr_signal_mask, SIGUSR1);

    struct sigaction SIGNAL_action;
    memset(&SIGNAL_action, 0, sizeof(struct sigaction));

    SIGNAL_action.sa_handler = handler;
    SIGNAL_action.sa_mask = int_quit_chld_usr_signal_mask;
    SIGNAL_action.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGINT, &SIGNAL_action, NULL);
    sigaction(SIGQUIT, &SIGNAL_action, NULL);
    sigaction(SIGUSR1, &SIGNAL_action, NULL);
    sigaction(SIGCHLD, &SIGNAL_action, NULL);

    ////// BLOCK signals of interest critical part of process //////
    sigprocmask(SIG_BLOCK, &int_quit_chld_usr_signal_mask, NULL);

    // aggragator
    Worker* worker;
    int num_of_workers;
    int* pipe_id;
    char** directories;
    int num_of_directories;
    char* input_dir;
    char* buffer_size_string;

    // server ip
    char* server_ip;
    char* server_port;

    // pipes
    int* write_fds;
    int* read_fds;
    int *previous_offset;
    Buffer* buffer;
    int buffer_size;

    // queries
    int success = 0;
    int fail = 0;

    int err = 0;
    for (int i = 1; i < argc; i++){

        if(strcmp(argv[i],"-w") == 0){
            num_of_workers = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i],"-i") == 0){
            input_dir = argv[i+1];
        }
        else if(strcmp(argv[i],"-b") == 0){
            buffer_size = atoi(argv[i+1]);
            buffer_size_string = argv[i+1];
        }
        else if(strcmp(argv[i],"-s") == 0){
            server_ip = argv[i+1];
        }
        else if(strcmp(argv[i],"-p") == 0){
            server_port = argv[i+1];
        }
        
    }
    // printf("\n dir-> %s  workers-> %d buffersize-> %d\n", input_dir, num_of_workers, buffer_size);

    // get directories
    get_directory_content(input_dir, DT_DIR, &num_of_directories, &directories);
    
    // if there are more workers thna directories then limit the num of workers
    if( num_of_directories < num_of_workers )
        num_of_workers = num_of_directories;

    // init workers
    Worker_Init(&worker, num_of_workers);

    // init server info
    Worker_add_server_info(worker, num_of_workers, server_ip, server_port);
    
    // assign directories to workers with Round Robin method
    get_directories_per_worker_RR(worker, num_of_workers, directories, num_of_directories);


    // init pipes
    char pipe_id_char[32] = {0};
    pipe_id = malloc(num_of_workers*sizeof(int));
    for (int i = 0; i < num_of_workers; i++)
        pipe_id[i] = i;

    // create pipes with unique id
    create_pipes(pipe_id, num_of_workers);
    
    // init buffer and offsets for pipes communication
    Message_Create_buffer_offset(&buffer, &previous_offset, num_of_workers);

    // IPC part begins

    // fork workers
    fork_workers(worker, 0, num_of_workers, buffer_size_string, pipe_id, input_dir);
    

    // create 2 pipes per worker for reading and writing from aggragator to worker and in reverse
    // pipes are named worker.(pid) aggragator.(pid)
    open_pipes(&read_fds, "master", O_RDONLY | O_NONBLOCK, pipe_id, num_of_workers);
    open_pipes(&write_fds, "worker", O_WRONLY , pipe_id, num_of_workers);

    // send directories to workers via pipe
    write_directories_to_workers(worker, 0, num_of_workers, write_fds, buffer_size);
    
    // send server ip and port
    write_server_info_to_workers(worker, 0, num_of_workers, write_fds, buffer_size, server_ip, server_port);

    // get the statistics from the workers
    // read_statistics_from_workers(0, num_of_workers, read_fds, buffer, previous_offset, buffer_size);
    
   

    char string_command[READ_BUFFER_SIZE];
    char intermediate_buffer[buffer_size];
    Message_vector command;
    Message_Init(&command);
    int query_status = 0;
    int bytes_read = 0;
    int total_bytes_read = 0;
    int select_output = 0;
    int num_of_args = 0;
    fd_set fd_set;  FD_ZERO(&fd_set); FD_SET(STDOUT_FILENO, &fd_set);
    // set timer
    struct timeval timeval; timeval.tv_sec = TIMEOUT_SEC;  timeval.tv_usec = TIMEOUT_USEC;

    // Unblock codes critical part
    sigprocmask(SIG_UNBLOCK, &int_quit_chld_usr_signal_mask, NULL);

    // start the queries
    do{
        // // get and define the arguments of the commands
        // // read command from stdin
        // // use select and read to interupt for singlal handler
        // memset(string_command, 0, READ_BUFFER_SIZE);
        // get_fd_set(&fd_set, read_fds, num_of_workers); FD_SET(0, &fd_set);
        // timeval.tv_sec = TIMEOUT_SEC;  timeval.tv_usec = TIMEOUT_USEC;
        // total_bytes_read = 0;
        // num_of_args = 0;
        // while(signal_occured == 0 && num_of_args == 0 ){

        //     select_output = select(STDOUT_FILENO+1, &fd_set, NULL, NULL, &timeval);

        //     if (select_output > 0 && signal_occured == 0 /*&& FD_ISSET(STDIN_FILENO, &fd_set) */){
        //         while(   (bytes_read = read(STDIN_FILENO, intermediate_buffer, buffer_size)) > 0 ){
        //             // copy read bytes
        //             memcpy(string_command+total_bytes_read, intermediate_buffer, bytes_read );
        //             total_bytes_read += bytes_read;
        //             string_command[total_bytes_read] = '\0';
        //             // scan for end of line
        //             num_of_args = number_of_args(string_command);
        //             if ((string_command[total_bytes_read-1] == '\n' && num_of_args > 0)  || signal_occured == 1)
        //                 break;
        //             memset(intermediate_buffer, 0, buffer_size);
        //         }
        //     }
        //     else{
        //         // reset timer for long awaited input
        //         FD_ZERO(&fd_set); FD_SET(STDOUT_FILENO, &fd_set);
        //         timeval.tv_sec = TIMEOUT_SEC;  timeval.tv_usec = TIMEOUT_USEC;
        //     }

        // }

        // // if any signal happens here then wait for the query to be completed and then sigkill the workers
        // // queries
        // // only if no signal has occured
        // queries_started = 1;
        // if ( signal_occured == 0){
        //     Message_to_vector(string_command, &command);
        //     exit_status = queries(&command, worker, num_of_workers, write_fds, read_fds, buffer_size, &success, &fail);
        //     Message_Delete(&command);
        //     fflush(stdout);
        // }
        // queries_started = 0;

        // pause and wait for a signal
        pause();
 
        
        // signal handling flag
        if( signal_occured == 1){
            // block all incomign signals during signal handlers
            sigprocmask(SIG_BLOCK, &int_quit_chld_usr_signal_mask, NULL);

            // process signal one by one from the signal queue
            while ( ( signal_status = pop_signal(signal_buffer, signal_sent_from_pid_buffer, &signal_sent_from_pid) ) > 0 ){
            
                printf("Handling signal: %d from: %d\n",signal_status, signal_sent_from_pid);

                // sigint kill all children
                if ( signal_status == SIGINT || signal_status == SIGQUIT ){
                    SIGINT_function(&worker, num_of_workers, &directories, num_of_directories, success, fail, &buffer, &previous_offset, &write_fds, &read_fds, &pipe_id);
                }

                // childer died it needs to be replaced
                if ( signal_status == SIGCHLD ){
                    SIGCHLD_function(worker, num_of_workers, write_fds, read_fds, buffer, previous_offset, pipe_id, buffer_size, buffer_size_string, input_dir);
                }

                // sigusr1 new file added
                if ( signal_status == SIGUSR1 ){
                    SIGUSR1_function(worker, num_of_workers, read_fds, buffer, previous_offset, buffer_size, signal_sent_from_pid);
                }

            }

            // reset signal flag
            signal_occured = 0;
            // end signal blocking
            sigprocmask(SIG_UNBLOCK, &int_quit_chld_usr_signal_mask, NULL);
        }

    } while(exit_status == 0);

    // block sigchild
    sigprocmask(SIG_BLOCK, &int_quit_chld_usr_signal_mask, NULL);
    kill_workers(worker, num_of_workers);
    wait_workers(worker, num_of_workers);
    print_log(directories, num_of_directories, success, fail);
    free_master(&worker, num_of_workers, &read_fds, &write_fds, &buffer, &previous_offset, &directories, num_of_directories, &pipe_id);
    exit(0);
}





