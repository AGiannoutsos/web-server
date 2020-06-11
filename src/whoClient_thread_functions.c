#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>

#include "../includes/whoClient_thread_functions.h"

pthread_mutex_t cbuffer_mutex;
pthread_cond_t cbuffer_empty_condition;
pthread_cond_t cbuffer_full_condition;

void *thread_connection_handler(void *args){
    //!!// mutex down start_query

    // mutex down new_thread 
    // threads_online++
    // mutex up new_thread
    // signal_new_thread

    // mutex down start_quety
    // while{threads_online < thread_id_x
    //      wait_cond_start_query}
    // mutex_up start_query


    int err = 0;
    char* query = (char*)args;
    
    // usleep(500000);
    printf(">>%d> %s", pthread_self()%20 , query);



    // exitng thred detaching
    if( err = pthread_detach(pthread_self()) ) {
        perror_t("Thread Ditach error ", err);
        exit(1);
    }
    // exiting
    pthread_exit(NULL);
}