#ifndef MASTER_FUNCTIONS_H
#define MASTER_FUNCTIONS_H

#include "../includes/master_functions.h"
#include "../includes/queries_master.h"
#include "../includes/message_handlers.h"
#include "../includes/util.h"

extern int exit_status;
extern volatile sig_atomic_t signal_status;

// wait workers to close after signal
int wait_workers(Worker* worker, int num_of_workers);
// free all master memory
void free_master(Worker** worker, int num_of_workers, int** read_fds, int** write_fds, Buffer** buffer, int** previous_offset, char*** directories, int num_of_directories, int** pipe_id);
// print log file
void print_log(char** directories, int num_of_directories, int success, int fail);
// kill workers with sigkill
void kill_workers(Worker* worker, int num_of_workers);
// for i number of workers
int fork_workers(Worker* worker, int i, int num_of_workers, char* buffer_size_string, int* pipe_id, char* input_dir);
// send directories to workers
void write_directories_to_workers(Worker* worker, int i, int num_of_workers, int* write_fds, int buffer_size);
// read i number of workers statistivs
void read_statistics_from_workers(int i, int num_of_workers, int* read_fds, Buffer* buffer, int* previous_offset, int buffer_size);
// queries
int queries(Message_vector* command, Worker* worker, int num_of_workers, int* write_fds, int* read_fds, int buffer_size, int* success, int* fail);

// signal actions
void SIGCHLD_function(Worker* worker, int num_of_workers, int* write_fds, int* read_fds, Buffer* buffer, int* previous_offset, int* pipe_id, int buffer_size, char* buffer_size_string, char* input_dir);
void SIGINT_function(Worker** worker, int num_of_workers, char*** directories, int num_of_directories, int success, int fail, Buffer** buffer, int** previous_offset, int** write_fds, int** read_fds, int** pipe_id);
void SIGUSR1_function(Worker* worker, int num_of_workers, int* read_fds, Buffer* buffer, int* previous_offset, int buffer_size, int worker_pid);



#endif