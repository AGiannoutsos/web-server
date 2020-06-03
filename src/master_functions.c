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

int fork_workers(Worker* worker, int i, int num_of_workers, char* buffer_size_string, int* pipe_id, char* input_dir){
    // fork workers
    // but also can for 1 worker if i = num_of_workers
    pid_t pid;
    char pipe_id_char[32];
    int err = 0;
    for (; i < num_of_workers; i++){

        pid = fork();
        if (pid < 0){
            perror("fork");
            exit(1);
        }

        // parent
        if (pid > 0){
            worker[i].pid = pid;
        }

        // child process
        if (pid == 0){
            // pass pipe id
            sprintf(pipe_id_char, "%d", pipe_id[i]);
            err = execl("./worker", "./worker", buffer_size_string, pipe_id_char, input_dir, NULL);
            if(err < 0){
                perror("execl");
                exit(1);
            }
            memset(pipe_id_char, 0, 32);
        }
    }
    return err;
}

void write_directories_to_workers(Worker* worker, int i, int num_of_workers, int* write_fds, int buffer_size){
    // can also send to only one worker directories
    Message_vector temp_message;
    for (; i < num_of_workers; i++){
        // copy directory to temp message
        temp_message.num_of_args = worker[i].num_of_directories;
        temp_message.args = malloc(worker[i].num_of_directories*sizeof(char*));

        for (int j = 0; j < worker[i].num_of_directories; j++){
            temp_message.args[j] = malloc((strlen(worker[i].directories[j])+1)*sizeof(char));
            strcpy(temp_message.args[j], worker[i].directories[j]);
        }
        Message_Write(write_fds[i], &temp_message, buffer_size);
        // Message_Print(&temp_message);
        Message_Delete(&temp_message);
    }
}

void read_statistics_from_workers(int i, int num_of_workers, int* read_fds, Buffer* buffer, int* previous_offset, int buffer_size){
    // can also read from one worker statistics
    Message_vector statistics;
    Message_Init(&statistics);
    char inter[buffer_size];

    // read from every worker
    // end if worker has sent End of Communication
    for (; i < num_of_workers; i++){
        previous_offset = Message_Read_from_one(read_fds, i, &statistics, buffer, previous_offset, buffer_size);
        while(!Message_Is_End_Com(&statistics, 1)){
            Statistics_Print(&statistics, stdout);
            previous_offset = Message_Read_from_one(read_fds, i, &statistics, buffer, previous_offset, buffer_size);
        }
        read(read_fds[i], inter, buffer_size);
        previous_offset[i] = 0;
    }
    Message_Delete(&statistics);
}

void SIGUSR1_function(Worker* worker, int num_of_workers, int* read_fds, Buffer* buffer, int* previous_offset, int buffer_size, int worker_pid){

    // // find which pipes have available data
    // struct timeval timeval; timeval.tv_sec = TIMEOUT_SEC;  timeval.tv_usec = TIMEOUT_USEC*2;
    // fd_set fd_set;
    // get_fd_set(&fd_set, read_fds, num_of_workers);

    // select(max(read_fds, num_of_workers)+1, &fd_set, NULL, NULL, &timeval);

    // for (int pipe = 0; pipe < num_of_workers; pipe++){
    //     if ( FD_ISSET(read_fds[pipe], &fd_set) ){
    //         // if pipe has available data then find the pipe id and read the statistics
    //         read_statistics_from_workers(pipe, pipe+1, read_fds, buffer, previous_offset, buffer_size);
    //     }
    // }
    int worker_position = 0;
    for (int i = 0; i < num_of_workers; i++){
        if (worker[i].pid == worker_pid){
            worker_position = i;
            break;
        }
    }
    // read statistics from the worker that send signal
    read_statistics_from_workers(worker_position, worker_position+1, read_fds, buffer, previous_offset, buffer_size);
}


void SIGINT_function(Worker** worker, int num_of_workers, char*** directories, int num_of_directories, int success, int fail, Buffer** buffer, int** previous_offset, int** write_fds, int** read_fds, int** pipe_id){
    // kill workers print log and free memory
    kill_workers(worker[0], num_of_workers); 
    wait_workers(worker[0], num_of_workers);
    print_log(directories[0], num_of_directories, success, fail);
    free_master(worker, num_of_workers, read_fds, write_fds, buffer, previous_offset, directories, num_of_directories, pipe_id);
    exit(signal_status);
}

void SIGCHLD_function(Worker* worker, int num_of_workers, int* write_fds, int* read_fds, Buffer* buffer, int* previous_offset, int* pipe_id, int buffer_size, char* buffer_size_string, char* input_dir) {
    int err;
    pid_t terminaterd_worker;
    int terminaterd_worker_position = 0;
    int status = 0;

    // wait for multiple chlidren in case that signlas SIGCHLD didnt reached the parent
    for (int child = 0; child < num_of_workers; child++){

        status = 0;
        // terminaterd_worker = wait(&status);
        // wait until you find a zombie
        terminaterd_worker = waitpid(worker[child].pid, &status, WNOHANG);

        // if child terminated normally then handler has nothing to do
        if (status != 0){

            printf("Child has finished: PID = %d\n", terminaterd_worker);
            if(WIFEXITED(status))
                printf("Child exited with code %d\n", status);
            else
                printf("Child terminated abnormally %d\n",(status));


            
            // find the position of terminated worker in workers array
            for (int i = 0; i < num_of_workers; i++){
                if (worker[i].pid == terminaterd_worker){
                    terminaterd_worker_position = i;
                    break;
                }
            }
            // close and reopen broken pipes
            char pipe_name_master[64] = {0};
            char pipe_name_worker[64] = {0};
            sprintf(pipe_name_master, "master.%d", pipe_id[terminaterd_worker_position]);
            sprintf(pipe_name_worker, "worker.%d", pipe_id[terminaterd_worker_position]);

            // printf("close piee-> %s  %d\n",pipe_name_master, read_fds[terminaterd_worker_position]);
                close(read_fds[terminaterd_worker_position]);
                if (read_fds[terminaterd_worker_position] < 0)
                    perror("close pipe");
            
            // printf("close piee-> %s  %d\n",pipe_name_worker, write_fds[terminaterd_worker_position]);
                close(write_fds[terminaterd_worker_position]);
                if (write_fds[terminaterd_worker_position] < 0)
                    perror("close pipe");

            fork_workers(worker, terminaterd_worker_position, terminaterd_worker_position+1, buffer_size_string, pipe_id, input_dir);

            // printf("open piee-> %s\n",pipe_name_master);
            read_fds[terminaterd_worker_position] = open(pipe_name_master, O_RDONLY | O_NONBLOCK);
            if (read_fds[terminaterd_worker_position] < 0)
                perror("open pipe");

            // printf("open piee-> %s\n",pipe_name_worker);
            write_fds[terminaterd_worker_position] = open(pipe_name_worker, O_WRONLY);
            if (write_fds[terminaterd_worker_position] < 0)
                perror("open pipe");

            // send worker directories
            write_directories_to_workers(worker, terminaterd_worker_position, terminaterd_worker_position+1, write_fds, buffer_size);

            // receive statistics
            read_statistics_from_workers(terminaterd_worker_position, terminaterd_worker_position+1, read_fds, buffer, previous_offset, buffer_size);
            
        }
    }
}


int queries(Message_vector* command, Worker* worker,  int num_of_workers, int* write_fds, int* read_fds, int buffer_size, int* success, int* fail){
    int query_status = 0;
    if(strcmp(command->args[0], "/listCountries") == 0){
        // printf("Start query of listCountries\n\n");
        list_countries(worker, num_of_workers);
        (*success)++;
    }
    else if(strcmp(command->args[0], "/diseaseFrequency") == 0){
        // printf("Start query of diseaseFrequency\n\n");
        query_status = disease_frequency(write_fds, read_fds, num_of_workers, command, buffer_size);
        if(query_status > 0)
            (*success)++;
        else
            (*fail)++;
        // sleep(5);
    }
    else if(strcmp(command->args[0], "/topk-AgeRanges") == 0){
        // printf("Start query of topk-AgeRanges\n\n");
        query_status = topk_age_ranges(write_fds, read_fds, num_of_workers, command, buffer_size);
        if(query_status > 0)
            (*success)++;
        else
            (*fail)++;
    }
    else if(strcmp(command->args[0], "/searchPatientRecord") == 0){
        // printf("Start query of searchPatientRecord\n\n");
        query_status = search_patient_record(write_fds, read_fds, num_of_workers, command, buffer_size);
        if(query_status > 0)
            (*success)++;
        else
            (*fail)++;
    }
    else if(strcmp(command->args[0], "/numPatientAdmissions") == 0){
        // printf("Start query of numPatientAdmissions\n\n");
        query_status = num_patients_admissions_discharges(write_fds, read_fds, num_of_workers, command, buffer_size);
        if(query_status > 0)
            (*success)++;
        else
            (*fail)++;
    }
    else if(strcmp(command->args[0], "/numPatientDischarges") == 0){
        query_status = num_patients_admissions_discharges(write_fds, read_fds, num_of_workers, command, buffer_size);
        if(query_status > 0)
            (*success)++;
        else
            (*fail)++;

    }
    else if(strcmp(command->args[0], "/exit") == 0 || exit_status == 1){
        printf("exiting\n");
        for (int i = 0; i < num_of_workers; i++){
            Message_Write(write_fds[i], command, buffer_size);
        }
        Message_Delete(command);


        return 1;
    }
    else{
        printf("error\n");
    }
    return 0;
}

void print_log(char** directories, int num_of_directories, int success, int fail){
    // cra=eate and print logs to log file
    char log_file_name[32] = {0};
    sprintf(log_file_name, "log_file.%d", getpid());
    FILE* log_file = fopen(log_file_name, "w");

    for (int i = 0; i < num_of_directories; i++){
        fprintf(log_file, "%s\n", directories[i]);
    }
    fprintf(log_file, "TOTAL %d\n", success+fail);
    fprintf(log_file, "SUCCESS %d\n", success);
    fprintf(log_file, "FAIL %d\n", fail);
    fclose(log_file);
}

void kill_workers(Worker* worker, int num_of_workers){
    for (int i = 0; i < num_of_workers; i++){
        kill(worker[i].pid, SIGKILL);
    }
}

int wait_workers(Worker* worker, int num_of_workers){
    // close all child workers
    int status = 0;
    pid_t terminaterd_worker;
    for (int i = 0; i < num_of_workers; i++){
        terminaterd_worker = waitpid(worker[i].pid, &status, 0);
        printf("Child has finished: PID = %d\n", terminaterd_worker);
        if(WIFEXITED(status))
            printf("Child exited with code %d\n", status);
        else
            printf("Child terminated abnormally %d\n",(status));
    }
    return status;
}

void free_master(Worker** worker, int num_of_workers, int** read_fds, int** write_fds, Buffer** buffer, int** previous_offset, char*** directories, int num_of_directories, int** pipe_id){
    // free aggragator stuff
    Message_Destroy_buffer_offset(buffer, previous_offset, num_of_workers);
    close_pipes(read_fds, "master", *pipe_id, num_of_workers);
    close_pipes(write_fds, "worker", *pipe_id, num_of_workers);
    unlink_pipes(*pipe_id, num_of_workers);
    Worker_Destroy(worker, num_of_workers);
    for (int i = 0; i < num_of_directories; i++){
        free(directories[0][i]);
    }
    free(*directories);
    free(*pipe_id);
}