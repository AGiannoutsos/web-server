#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>

#include "../includes/whoServer_thread_functions.h"
#include "../includes/circular_buffer.h"

void *thread_connection_handler(void *args){
    int err = 0;
    Thread_Args* thread_args = (Thread_Args*)args;
    Circular_Buffer* circular_buffer = thread_args->circular_buffer;

    // get socket from buffer
    // secure with semaphores
    int a,aa;
    while( 1 ){
        CBUFFER_Pop(circular_buffer, &a, &aa);
        printf("poped %d %d\n", a, aa);
        sleep(5);
    }


    // exitng thred detaching
    if( err = pthread_detach (pthread_self()) ) {
        perror_t("Thread Ditach error ", err);
        exit(1);
    }

}