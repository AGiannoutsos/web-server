#ifndef WHOSERVER_THREAD_FUNCTIONS_H
#define WHOSERVER_THREAD_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>

#include "../includes/circular_buffer.h"

// mutexes
extern pthread_mutex_t cbuffer_mutex;
extern pthread_cond_t cbuffer_empty_condition;
extern pthread_cond_t cbuffer_full_condition;

// error function
#define perror_t(string,error) fprintf( stderr,"%s: %s\n", string, strerror(error))
#define check_t(error, string) if(error != 0) { perror_t(string, error);  exit(error); }


// arguments for thread
typedef struct Thread_Args{

    Circular_Buffer* circular_buffer; 
    int counter;

} Thread_Args;


void *thread_connection_handler(void *args);

int statistics_connection_handler(int statistics_socket);

int queries_connection_handler(int queries_socket);


#endif