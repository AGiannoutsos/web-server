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
    pthread_mutex_t* print_mutex = thread_args->print_mutex;
    //!!// mutex down start_query

    // mutex down new_thread 
    // threads_online++
    // mutex up new_thread
    // signal_new_thread

    // mutex down start_quety
    // while{threads_online < thread_id_x
    //      wait_cond_start_query}
    // mutex_up start_query


    // usleep(pthread_self()%90000);

    pthread_mutex_lock(&threads_online_mutex);
    threads_online++;
    pthread_mutex_unlock(&threads_online_mutex);
    pthread_cond_signal(&threads_online_condition);


    pthread_mutex_lock(&start_queries_mutex);
    while( start_queries != 1){
        // printf("cant start query yet\n\n");
        pthread_cond_wait(&start_queries_condition, &start_queries_mutex);
    }
    pthread_mutex_unlock(&start_queries_mutex);

    // now that all threads are online its time to connect with the server
    int server_socket = SOCKET_Connect(AF_INET, SOCK_STREAM, server_port, server_ip);
    check(server_socket, "Connection with server failed");
    printf("s %d\n",server_socket);
    

    // set socket non-block
    // check( fcntl(server_socket, F_SETFL, fcntl(server_socket, F_GETFL, 0) | O_NONBLOCK), "Socket Non-block error");
    
    // sent query to server
    // create message vector
    Message_vector query_message;
    Message_vector results_message;
    Message_Init(&query_message);
    Message_Init(&results_message);
    // create buffer for results
    Buffer *results_buffer;
    int *results_previous_offset;
    Message_Create_buffer_offset(&results_buffer, &results_previous_offset, 1);


    // construct message
    query_message.num_of_args = 1;
    query_message.args = malloc(sizeof(char*));
    query_message.args[0] = malloc((strlen(query)+1)*sizeof(char));
    strcpy(query_message.args[0], query); 

    // sleep(1);
    // send query
    Message_Write(server_socket, &query_message, BUFFER_DEFAULT_SIZE);
    Message_Write_End_Com(server_socket, BUFFER_DEFAULT_SIZE);

    // sleep(1);
    // get results  and print safely
    results_previous_offset = Message_Read_from_one_socket(&server_socket, 0, &results_message, results_buffer, results_previous_offset, 100);
    while(!Message_Is_End_Com(&results_message, 1)){

        // block stdout with mutex so that results are printed smooth
        pthread_mutex_lock(print_mutex);
        printf("%s", query);
        Message_Print_results(&results_message);
        printf("\n");
        pthread_mutex_unlock(print_mutex);

        results_previous_offset = Message_Read_from_one_socket(&server_socket, 0, &results_message, results_buffer, results_previous_offset, 100);
    }

    // send server end of com so that alla data have been transfered and connection can be closed
    // Message_Write_End_Com(server_socket, BUFFER_DEFAULT_SIZE);
    // sleep(1);

    Message_Delete(&query_message);
    Message_Delete(&results_message);
    Message_Destroy_buffer_offset(&results_buffer, &results_previous_offset, 1);
    // printf("clo %d\n",server_socket);

    // printf(">(%d)>%d> %s %d %s", start_queries, pthread_self()%20 , query, server_port, server_ip);




    // if ( shutdown(server_socket, SHUT_RDWR) < 0)
    //     perror("Shutdown failed");

    // exitng thread detaching
    // exiting
    // usleep(pthread_self()%90000);

    pthread_mutex_lock(&threads_offline_mutex);
    threads_offline++;
    // printf("---------------------> offline %d\n",threads_offline);
    pthread_mutex_unlock(&threads_offline_mutex);
    pthread_cond_signal(&threads_offline_condition);

    close(server_socket);
    // if( err = pthread_detach(pthread_self()) ) {
    //     perror_t("Thread Detach error ", err);
    //     exit(1);
    // }
    // fprintf( stderr,"clo %d\n",server_socket);

    pthread_mutex_lock(&end_queries_mutex);
    while( end_queries != 1){
        // printf("cant end query yet\n\n");
        pthread_cond_wait(&end_queries_condition, &end_queries_mutex);
    }
    pthread_mutex_unlock(&end_queries_mutex);


    // printf("exitingggg \n");
    // pthread_exit(NULL);
    // return 0;
    // sleep(5);
    
}