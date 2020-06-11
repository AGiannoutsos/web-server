#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>

#include "../includes/whoServer_thread_functions.h"
#include "../includes/master_functions.h"
#include "../includes/message_handlers.h"
#include "../includes/circular_buffer.h"
#include "../includes/socket_functions.h"

#define BUFFER_DEFAULT_SIZE 100

pthread_mutex_t cbuffer_mutex;
pthread_cond_t cbuffer_empty_condition;
pthread_cond_t cbuffer_full_condition;

void *thread_connection_handler(void *args){
    int err = 0;
    Thread_Args* thread_args = (Thread_Args*)args;
    Circular_Buffer* circular_buffer = thread_args->circular_buffer;
    

    // get socket from buffer
    // secure with semaphores
    // for (int i = 0; i < 1000000; i++){
    //     check_t(pthread_mutex_lock(&cbuffer_mutex), "Thread mutex lock error");
    //     thread_args->counter++;
    //     check_t(pthread_mutex_unlock(&cbuffer_mutex), "Thread mutex unlock error");
    // }
    
    
    // int* socket = malloc(sizeof(int));
    // memset(socket, 0, sizeof(int));
    
    // int* type = malloc(sizeof(int));
    // memset(type, 0, sizeof(int));
    int socket = 0;
    int type = 0;
    while( 1 ){


        CBUFFER_Pop_sync(circular_buffer, &socket, &type);

        printf("poped %d %d      %d\n", socket, type, CBUFFER_Is_Empty(circular_buffer));
        // sleep(1);

        // set socket NONBLOCK
        err = fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) | O_NONBLOCK);
        check(err, "Socket Non-block error");

        // gather indormation about worker and statistics
        if (type == STAT_TYPE){
            printf("statistics handdler\n");
            statistics_connection_handler(socket);
        }
        else if (type == QERY_TYPE){
            printf("xixi query handler\n");
            queries_connection_handler(socket);
        }
        
    }


    // exitng thred detaching
    if( err = pthread_detach(pthread_self()) ) {
        perror_t("Thread Ditach error ", err);
        exit(1);
    }
    // exiting
    pthread_exit(NULL);
}

int statistics_connection_handler(int statistics_socket){
    int* previous_offset;
    Buffer* buffer;
    Message_vector port_message;
    Message_Init(&port_message);
    // allocate buffer for the message 
    Message_Create_buffer_offset(&buffer, &previous_offset, 1);

    read_statistics_from_workers(0, 1, &statistics_socket, buffer, previous_offset, BUFFER_DEFAULT_SIZE);

    // read worker socket port
    previous_offset = Message_Read_from_one(&statistics_socket, 0, &port_message, buffer, previous_offset, BUFFER_DEFAULT_SIZE);
        while(!Message_Is_End_Com(&port_message, 1)){
            // printf("fdd-> %d port-------------------------->> %s\n", statistics_socket, port_message.args[0]);
            previous_offset = Message_Read_from_one(&statistics_socket, 0, &port_message, buffer, previous_offset, BUFFER_DEFAULT_SIZE);
        }

    // deallocate buffer
    Message_Destroy_buffer_offset(&buffer, &previous_offset, 1);
    printf("\n\nStatistivs endedddd!! fd %d   port _> %s\n\n\n", statistics_socket, port_message.args[0]);

    Message_Delete(&port_message);
}


int queries_connection_handler(int queries_socket){
    FILE* sock_stream = fdopen(queries_socket, "r+");
    // listen to client
    char buf[100] = {0};
    fgets(buf, 100, sock_stream);
    printf("%s\n",buf);
    if ( shutdown(queries_socket, SHUT_RDWR) < 0)
        perror("Shutdown failed");
    printf("\n\nStatistivs endedddd!!\n");
}