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

// sockets
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "../includes/worker_functions.h"

#include "../includes/message_handlers.h"
#include "../includes/util.h"
#include "../includes/hashtable.h"
#include "../includes/patient_list.h"
#include "../includes/avl_tree.h"
#include "../includes/ht_avlt.h"
#include "../includes/queries_worker.h"
#include "../includes/socket_functions.h"



#define MAX_FILE_PATH 515
#define IP_ADDRESS "0.0.0.0"

extern int errno;

void handler(int sig, siginfo_t *info, void *ucontext);
void queries(Message_vector* command, int* success, int* fail, Worker* me_worker, int* write_fd, int buffer_size, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table);
// same with Message_Read_from_one from signal handler but with signal handling
int* Message_Read_from_one_signal(int* fd, int num_of_fd, Message_vector* _message, Buffer* buffer, int* previous_offset, int buffer_size);


int exit_status = 0;

volatile sig_atomic_t signal_sent_from_pid = 0;
volatile sig_atomic_t signal_occured = 0;
volatile sig_atomic_t signal_status = 0;



void handler(int sig, siginfo_t *info, void *ucontext){
    signal_sent_from_pid = info->si_pid;
    signal_occured = 1;
    signal_status = sig;
}

int main(int arc, char** argv){

    // signals init
    sigset_t fullmask, int_quit_usr_signal_mask;

    sigfillset(&fullmask);
    // reset any blockd signal 
    sigprocmask(SIG_UNBLOCK, &fullmask, NULL);


    sigemptyset(&int_quit_usr_signal_mask);
    sigaddset(&int_quit_usr_signal_mask, SIGINT);
    sigaddset(&int_quit_usr_signal_mask, SIGQUIT);
    sigaddset(&int_quit_usr_signal_mask, SIGUSR1);


    struct sigaction SIGNAL_action;
    memset(&SIGNAL_action, 0, sizeof(struct sigaction));

    SIGNAL_action.sa_handler = handler;
    SIGNAL_action.sa_mask = int_quit_usr_signal_mask;
    SIGNAL_action.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGINT, &SIGNAL_action, NULL);
    sigaction(SIGQUIT, &SIGNAL_action, NULL);
    sigaction(SIGUSR1, &SIGNAL_action, NULL);

    // BLOCK signals of interest critical part of process
    sigprocmask(SIG_BLOCK, &int_quit_usr_signal_mask, NULL);


    int buffer_size;
    int pipe_id;
    char* input_dir;
    Worker* me_worker;

    // pipes
    int* read_fd;
    int* write_fd;
    int* previous_offset;
    Buffer* buffer;

    // data structures
    Hash_Table_s files_hashtable;
    Patient_List patient_list;
    Hash_Table disease_hash_table;
    Hash_Table country_hash_table;

    // queries
    int success = 0;
    int fail = 0;
    int queries_started = 0;
    

    // inti worker
    buffer_size = atoi(argv[1]);
    pipe_id = atoi(argv[2]);
    input_dir = argv[3];
    Worker_Init(&me_worker, 1);
    me_worker->pid = getpid();
    // open pipe link to get inforamtion about directories
    open_pipes(&write_fd, "master", O_WRONLY, &pipe_id, 1);
    open_pipes(&read_fd, "worker", O_RDONLY | O_NONBLOCK, &pipe_id, 1);


    // read direcotries 
    Message_vector directories_message;
    Message_Init(&directories_message);
    Message_vector server_info_message;
    Message_Init(&server_info_message);
    Message_Create_buffer_offset(&buffer, &previous_offset, 1);
    // get message of directories
    previous_offset = Message_Read_from_one(read_fd, 0, &directories_message, buffer, previous_offset, buffer_size);
    // copy them on me_worker info
    get_directories_per_worker_RR(me_worker, 1, directories_message.args, directories_message.num_of_args);

    // get server info ip and port
    previous_offset = Message_Read_from_one(read_fd, 0, &server_info_message, buffer, previous_offset, buffer_size);
    Worker_add_server_info(me_worker, 1, server_info_message.args[0], server_info_message.args[1]);
    Message_Print(&server_info_message);

    // delete message
    Message_Delete(&directories_message);
    Message_Delete(&server_info_message);






    // create socket to accept connections
    int queries_socket = SOCKET_Create(AF_INET, SOCK_STREAM, 0, IP_ADDRESS, LISTEN_DEFAULT);
    check(queries_socket, "Socket problem");
    printf("socket-> %d\n",queries_socket);

    // get the random available port number
    struct sockaddr_in connection_addres;
    memset(&connection_addres, 0, sizeof(connection_addres));
    socklen_t connection_addres_len = sizeof(connection_addres);

    check( getsockname(queries_socket, (struct sockaddr*) &connection_addres, &connection_addres_len), "Socket get name error" );
    int queries_port = ntohs(connection_addres.sin_port);


    // connect with server
    int server_socket = SOCKET_Connect(AF_INET, SOCK_STREAM, atoi(me_worker->server_port), me_worker->server_ip);
    check(server_socket, "Server connection failed");
    printf("\n port numberr %d  %d\n\n", queries_port, server_socket);




    // read files and store them in a hashtable and in all the data structures
    HT_Init_simple(&files_hashtable, 5, 512);
    PL_Init(&patient_list);
    HT_AVLT_Init(&disease_hash_table, 5, 512);
    HT_AVLT_Init(&country_hash_table, 5, 512);
    
    // store in all the data structures
    get_files_in_data_structures_send_statistics(server_socket, buffer_size, input_dir, me_worker->directories , me_worker->num_of_directories, &files_hashtable, &patient_list, &disease_hash_table, &country_hash_table);


    // send socket queries port number to server and ip address
    char port_string[64] = {0}; Message_vector queries_port_message; Message_Init(&queries_port_message);
    queries_port_message.num_of_args = 2; queries_port_message.args = malloc(2*sizeof(char*));
    sprintf(port_string, "%d", queries_port); queries_port_message.args[0] = malloc((strlen(port_string)+1)*sizeof(char));
    strcpy(queries_port_message.args[0], port_string);
    queries_port_message.args[1] = malloc((strlen(IP_ADDRESS)+1)*sizeof(char));
    strcpy(queries_port_message.args[1], IP_ADDRESS);

    Message_Write(server_socket, &queries_port_message, buffer_size);
    Message_Write_End_Com(server_socket, buffer_size); 

    // end communication with statistivs port of server
    check( shutdown(server_socket, SHUT_RDWR), "Shutdown error");
    close(server_socket);


    struct sockaddr_in connection_addres_;
    socklen_t connection_addres_len_ = 0;
    int queries_fd = 0;
    int num_of_args = 0;
    int query_status = 0;
    Message_vector command;
    Message_Init(&command);

    // Unblock codes critical part
    sigprocmask(SIG_UNBLOCK, &int_quit_usr_signal_mask, NULL);

    // start accepting queries
    do{
        printf("ip %s port %s\n",me_worker->server_ip, me_worker->server_port);
        // delete previous buffer
        Message_Destroy_buffer_offset(&buffer, &previous_offset, 1);

        // listen for server requests  
        queries_fd = accept(queries_socket, (struct sockaddr*) &connection_addres_, &connection_addres_len_);
        check(queries_fd, "Accept failed");

        // set socket non-block
        check( fcntl(queries_fd, F_SETFL, fcntl(queries_fd, F_GETFL, 0) | O_NONBLOCK), "Socket Non-block error");

        // create new buffer for this only query
        Message_Create_buffer_offset(&buffer, &previous_offset, 1);

        // Listen command from master
        previous_offset = Message_Read_from_one_signal(&queries_fd, 0, &command, buffer, previous_offset, buffer_size);

        queries_started = 1;

        Message_Print(&command);
        // start query only if no signal has arrived
        if ( signal_occured == 0){
            queries(&command, &success, &fail, me_worker, &queries_fd, buffer_size, &patient_list, &disease_hash_table, &country_hash_table);
        }
        
        if ( signal_occured == 1){

            if ( signal_status == SIGINT || signal_status == SIGQUIT ){   
                // kill worker deallocate memory print log and exit
                Message_Delete(&command);
                print_log(me_worker, success, fail);
                free_worker(&me_worker, &write_fd, &read_fd, &buffer, &previous_offset, pipe_id, &patient_list, &disease_hash_table, &country_hash_table, &files_hashtable);
                signal_occured = 0;
                exit(signal_status);
            }

            if ( signal_status == SIGUSR1 ){
                // but fisrt inform parent for statistics recieval
                kill(getppid(), SIGUSR1);
                // scan the folders and send statistics
                get_files_in_data_structures_send_statistics(write_fd[0], buffer_size, input_dir, me_worker->directories , me_worker->num_of_directories, &files_hashtable, &patient_list, &disease_hash_table, &country_hash_table);

            }
            signal_occured = 0;
        }

        queries_started = 0;
        close(queries_fd);
    } while(exit_status == 0);



    free_worker(&me_worker, &write_fd, &read_fd, &buffer, &previous_offset, pipe_id, &patient_list, &disease_hash_table, &country_hash_table, &files_hashtable);
    exit(0);
}



void queries(Message_vector* command, int* success, int* fail, Worker* me_worker, int* write_fd, int buffer_size, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table){

    int query_status = 0;
    if(strcmp(command->args[0], "/diseaseFrequency") == 0){
        // printf("Start query of diseaseFrequency %d\n\n", getpid());
        query_status = write_disease_frequency(write_fd[0], buffer_size, command->num_of_args, command->args, patient_list, disease_hash_table);
        if(query_status > 0)
           (*success)++;
        else
            (*fail)++;
        // sleep(5);
        // /diseaseFrequency EVD 01-01-2000 01-01-2021 Greece
        // /diseaseFrequency EBOLA 01-01-2000 01-01-2009
    }
    else if(strcmp(command->args[0], "/topk-AgeRanges") == 0){
        // printf("Start query of topk-AgeRanges\n\n");
        query_status = write_topk_age_ranges(write_fd[0], buffer_size, command->num_of_args, command->args, patient_list, disease_hash_table, country_hash_table);
        if(query_status > 0)
            (*success)++;
        else
            (*fail)++;

        // /topk-AgeRanges 4 Greece COVID-19 01-01-2000 01-01-2021
        // /topk-AgeRanges 4 India COVID-19 01-01-2000 01-01-2021
        // /topk-AgeRanges 4 India COVID-19 02-05-2002 01-01-2021
        // /topk-AgeRanges 4 India HIV 25-04-2002 01-01-2021
    }
    else if(strcmp(command->args[0], "/searchPatientRecord") == 0){
        // printf("Start query of searchPatientRecord\n\n");
        query_status = write_search_patient_record(write_fd[0], buffer_size, command->num_of_args, command->args, patient_list);
        if(query_status > 0)
            (*success)++;
        else
            (*fail)++;

        // /searchPatientRecord 12023
    }
    else if(strcmp(command->args[0], "/numPatientAdmissions") == 0){
        // printf("Start query of numPatientAdmissions\n\n");
        query_status = write_num_patient_admissions(write_fd[0], buffer_size, me_worker, command->num_of_args, command->args, patient_list, disease_hash_table, country_hash_table);
        if(query_status > 0)
            (*success)++;
        else
            (*fail)++;

        // /numPatientAdmissions COVID-19 01-01-2000 01-01-2021 Greece
        // /numPatientAdmissions COVID-19 01-01-2000 01-01-2021 USA
        // /numPatientAdmissions EVD 01-01-2000 01-01-2021 Greece
        
    }
    else if(strcmp(command->args[0], "/numPatientDischarges") == 0){
        // printf("Start query of numPatientDischarges\n\n");
        query_status = write_num_patient_discharges(write_fd[0], buffer_size, me_worker, command->num_of_args, command->args, patient_list, disease_hash_table, country_hash_table);
        if(query_status > 0)
            (*success)++;
        else
            (*fail)++;
        // /numPatientDischarges EVD 01-01-2000 01-01-2021 Greece
        // /numPatientDischarges EVD 0-0-0 01-01-2021

    }
    else if(strcmp(command->args[0], "/exit") == 0){
        Message_Delete(command);
        exit_status = 1;
        pause();
    }
    else{
        printf("error\n");
    }
}




// same with Message_Read_from_one from signal handler but with signal handling
int* Message_Read_from_one_signal(int* fds, int num_of_fd, Message_vector* _message, Buffer* buffer, int* previous_offset, int buffer_size){
    int fd = num_of_fd;
    int bytes_read = 0;
    char* end_of_message;
    int* bytes_write = previous_offset;
    char inter_buffer[buffer_size];
    memset(inter_buffer, '\0', buffer_size);
    
    Message_vector message;
    Message_Init(&message);

    fd_set fd_set;
    get_fd_set(&fd_set, &fds[fd], 1);

    // set timer
    struct timeval timeval;
    timeval.tv_sec = 0;
    timeval.tv_usec = 0;

    // Select if pipe haw data to offer then sleep
    int select_output = 0;
    int exit_select = 0;
    while( exit_select == 0 && signal_occured == 0 ){

        // wait timer 200 ms to read the nonblock pipe
        select_output = select(fds[fd]+1, &fd_set, NULL, NULL, &timeval);

        // chech every pipe if which is avaoilable to read
        // if pipe is ready then select it to read
        if( select_output > 0 ){  
            // while bytes available in pipe read and store to an intermediate buffer
            // and then copy them on the inserted buffer
            bytes_write[fd] = Message_Read_string(fds[fd], bytes_write[fd], &message, &buffer[fd], buffer_size);
            // if read is unavailable but buffer is the get data from it
            // in case of large buffer size but small messages
            if(bytes_write[fd] > 0 ){
                Message_Delete(&message);
                Message_to_vector(buffer[fd].buffer, &message);
                if (Message_Decoder(&message) > 0){ 
                    end_of_message = strstr(buffer[fd].buffer, MSG_END);
                    end_of_message += strlen(MSG_END);
                    bytes_write[fd] = (int)(buffer[fd].buffer - end_of_message)+bytes_write[fd];
                    memmove(buffer[fd].buffer, end_of_message, bytes_write[fd]);
                    // clear the buffer from dirty bytes
                    memset(buffer[fd].buffer+bytes_write[fd], '\0', buffer[fd].buffer_size-bytes_write[fd]);
                    // reset offset;
                    exit_select = 1;
                }
            }
        }
        else{
            // reset timer for long awaited input
            get_fd_set(&fd_set, &fds[fd], 1);
            timeval.tv_sec = TIMEOUT_SEC;
            timeval.tv_usec = TIMEOUT_USEC;

            if(bytes_write[fd] > 1 ){
                Message_Delete(&message);
                Message_to_vector(buffer[fd].buffer, &message);
                // set timeout to 0 because there are still data in the buffer
                timeval.tv_sec = 0;
                timeval.tv_usec = 0;
                if (Message_Decoder(&message) > 0){ 
                    end_of_message = strstr(buffer[fd].buffer, MSG_END);
                    end_of_message += strlen(MSG_END);
                    bytes_write[fd] = (int)(buffer[fd].buffer - end_of_message)+bytes_write[fd];
                    memmove(buffer[fd].buffer, end_of_message, bytes_write[fd]);
                    // clear the buffer from dirty bytes
                    memset(buffer[fd].buffer+bytes_write[fd], '\0', buffer[fd].buffer_size-bytes_write[fd]);
                    // reset offset;
                    exit_select = 1;
                }
            }
        }

   }
    
    // copy messsage and remove MSG_END MSG_START 
    Message_Delete(_message);
    if ( signal_occured == 0 ) // if signal happend then dont copy the message
        Message_Copy(_message, &message, 1, 1);
    Message_Delete(&message);
    return bytes_write;
}