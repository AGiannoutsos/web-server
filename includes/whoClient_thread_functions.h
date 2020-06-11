#ifndef WHOCLIENT_THREAD_FUNCTIONS_H
#define WHOCLIENT_THREAD_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>


// mutexes for synchronization
extern pthread_mutex_t threads_online_mutex;
extern pthread_cond_t threads_online_condition;

extern pthread_mutex_t start_queries_mutex;
extern pthread_cond_t start_queries_condition;

extern pthread_mutex_t threads_offline_mutex;
extern pthread_cond_t threads_offline_condition;

extern pthread_mutex_t end_queries_mutex;
extern pthread_cond_t end_queries_condition;



// error function
#define perror_t(string,error) fprintf( stderr,"%s: %s\n", string, strerror(error))
#define check_t(error, string) if(error != 0) { perror_t(string, error);  exit(error); }


// arguments for thread
typedef struct Thread_Args{

    char* query; 
    int server_port;
    char* server_ip;

} Thread_Args;

void *thread_connection_handler(void *args);

#endif