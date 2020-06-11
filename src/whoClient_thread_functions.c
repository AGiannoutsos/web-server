#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>

#include "../includes/whoClient_thread_functions.h"
#include "../includes/socket_functions.h"
#include "../includes/message_handlers.h"

// mutexes for synchronization
pthread_mutex_t threads_online_mutex;
pthread_cond_t threads_online_condition;

pthread_mutex_t start_queries_mutex;
pthread_cond_t start_queries_condition;

extern int start_queries;
extern int threads_online;

extern int end_queries;
extern int threads_offline;

void *thread_connection_handler(void *args){
    // arguments 
    int err = 0;
    Thread_Args* thread_args = (Thread_Args*)args;
    // Circular_Buffer* circular_buffer = thread_args->circular_buffer;
    char* query = thread_args->query;
    char* server_ip = thread_args->server_ip;
    int server_port = thread_args->server_port;
    //!!// mutex down start_query

    // mutex down new_thread 
    // threads_online++
    // mutex up new_thread
    // signal_new_thread

    // mutex down start_quety
    // while{threads_online < thread_id_x
    //      wait_cond_start_query}
    // mutex_up start_query


    usleep(pthread_self()%90000);

    pthread_mutex_lock(&threads_online_mutex);
    threads_online++;
    pthread_mutex_unlock(&threads_online_mutex);
    pthread_cond_signal(&threads_online_condition);


    pthread_mutex_lock(&start_queries_mutex);
    while( start_queries != 1){
        printf("cant start query yet\n\n");
        pthread_cond_wait(&start_queries_condition, &start_queries_mutex);
    }
    pthread_mutex_unlock(&start_queries_mutex);

    // now that all threads are online its time to connect with the server
    int server_socket = SOCKET_Connect(AF_INET, SOCK_STREAM, server_port, server_ip);
    check(server_socket, "Connection with server failed");
    
    
    // sent query to server
    // create message vector
    Message_vector query_message;
    Message_Init(&query_message);

    query_message.num_of_args = 1;
    query_message.args = malloc(sizeof(char*));
    query_message.args[0] = malloc((strlen(query)+1)*sizeof(char));
    strcpy(query_message.args[0], query); 

    Message_Write(server_socket, &query_message, 100);
    Message_Write_End_Com(server_socket, 100);

    Message_Delete(&query_message);

    printf(">(%d)>%d> %s %d %s", start_queries, pthread_self()%20 , query, server_port, server_ip);




    // if ( shutdown(server_socket, SHUT_RDWR) < 0)
    //     perror("Shutdown failed");

    // exitng thread detaching
    if( err = pthread_detach(pthread_self()) ) {
        perror_t("Thread Detach error ", err);
        exit(1);
    }
    // exiting
    // usleep(pthread_self()%90000);

    pthread_mutex_lock(&threads_offline_mutex);
    threads_offline++;
    pthread_mutex_unlock(&threads_offline_mutex);
    pthread_cond_signal(&threads_offline_condition);


    pthread_mutex_lock(&end_queries_mutex);
    while( end_queries != 1){
        // printf("cant end query yet\n\n");
        pthread_cond_wait(&end_queries_condition, &end_queries_mutex);
    }
    pthread_mutex_unlock(&end_queries_mutex);


    // printf("exitingggg \n");
    // pthread_exit(NULL);
}