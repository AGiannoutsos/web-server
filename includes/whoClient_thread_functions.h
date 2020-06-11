#ifndef WHOCLIENT_THREAD_FUNCTIONS_H
#define WHOCLIENT_THREAD_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>


// error function
#define perror_t(string,error) fprintf( stderr,"%s: %s\n", string, strerror(error))
#define check_t(error, string) if(error != 0) { perror_t(string, error);  exit(error); }


// arguments for thread
typedef struct Thread_Args{

    char* query; 

} Thread_Args;

void *thread_connection_handler(void *args);

#endif