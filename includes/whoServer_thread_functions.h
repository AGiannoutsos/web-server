#ifndef WHOSERVER_THREAD_FUNCTIONS
#define WHOSERVER_THREAD_FUNCTIONS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>

#include "../includes/circular_buffer.h"

// error function
#define perror_t(string,error) fprintf( stderr,"%s: %s\n", string, strerror(error))

// arguments for thread
typedef struct Thread_Args{

    Circular_Buffer* circular_buffer; 

} Thread_Args;


void *thread_connection_handler(void *args);


#endif