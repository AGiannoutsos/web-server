#ifndef UTIL_H
#define UTIL_H

#include "../includes/message_handlers.h"


typedef struct Worker{
    pid_t pid;
    int num_of_directories;
    char** directories;
    char* server_ip;
    char* server_port;
} Worker;

typedef struct Worker_directories{
    pid_t pid;
    int num_of_directories;
    char** directories;
} Worker_directories;

// init worker struct
int Worker_Init(Worker** worker, int num_of_workers);
int Worker_Destroy(Worker** worker, int num_of_workers);
// init server info
int Worker_add_server_info(Worker* worker, int num_of_workers, char* server_ip, char* server_port);

int max(int *array, int items);
int sum(int *s, int n);
// init fd set
int get_fd_set(fd_set* fd_set, int* fds, int num_of_fds);
// get directory content based on the type flag
int get_directory_content(char* input_dir, int type, int*_num_of_directories, char*** _directories);
// Round robbin assign directories to workers
int get_directories_per_worker_RR(Worker* worker, int num_of_workers, char** directories, int num_of_directories);

// signal buffer
int add_signal(volatile sig_atomic_t* buffer, volatile sig_atomic_t* sender_pid_buffer, int signal, int pid);
volatile sig_atomic_t pop_signal(volatile sig_atomic_t* buffer, volatile sig_atomic_t* sender_pid_buffer, volatile sig_atomic_t* signal_sent_from_pid);

// statistics handling
void Statistics_Update(Message_vector* statistics, Message_vector* file_content, char* country, char* date);
void Statistics_Print(Message_vector* statistics, FILE* output);

// pipe open close create unlink
int create_pipes(int* pipe_id, int num_of_workers);
int unlink_pipes(int* pipe_id, int num_of_workers);
int open_pipes(int** _fds, char* pipe, int mode, int* pipe_id, int num_of_fds);
int close_pipes(int** fds, char* pipe, int* pipe_id, int num_of_fds);

#endif