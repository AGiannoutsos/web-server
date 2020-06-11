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
#include "../includes/master_functions.h"



pthread_mutex_t cbuffer_mutex;
pthread_cond_t cbuffer_empty_condition;
pthread_cond_t cbuffer_full_condition;


void *thread_connection_handler(void *args){
    int err = 0;
    Thread_Args* thread_args = (Thread_Args*)args;
    Circular_Buffer* circular_buffer = thread_args->circular_buffer;
    Port_Stack* workers_ports = thread_args->workers_ports;
    pthread_mutex_t* worker_stack_mutex = thread_args->worker_stack_mutex;
    

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
            statistics_connection_handler(socket, workers_ports, worker_stack_mutex);
        }
        else if (type == QERY_TYPE){
            printf("xixi query handler\n");
            queries_connection_handler(socket, workers_ports);
        }

        // close socket after use
        close(socket);
        
    }


    // exitng thred detaching
    if( err = pthread_detach(pthread_self()) ) {
        perror_t("Thread Ditach error ", err);
        exit(1);
    }
    // exiting
    pthread_exit(NULL);
}

int statistics_connection_handler(int statistics_socket, Port_Stack* workers_ports, pthread_mutex_t* worker_stack_mutex){
    int* previous_offset;
    Buffer* buffer;
    Message_vector port_message;
    Message_Init(&port_message);
    // allocate buffer for the message 
    Message_Create_buffer_offset(&buffer, &previous_offset, 1);

    read_statistics_from_workers(0, 1, &statistics_socket, buffer, previous_offset, BUFFER_DEFAULT_SIZE);

    // read worker socket port
    previous_offset = Message_Read_from_one(&statistics_socket, 0, &port_message, buffer, previous_offset, BUFFER_DEFAULT_SIZE);
    // store port of worker in a data structure
    // lock with mutex the data structure
    pthread_mutex_lock(worker_stack_mutex);
    PSTACK_Insert(workers_ports, atoi( port_message.args[0]), port_message.args[1] );
    int* a = PSTACK_Get_Stack(workers_ports);
    int k = 0;
    while (a[k] != 0){
        printf("portsss-----------------> %d  %s\n", a[k++], workers_ports->ip_address);
    }  
    pthread_mutex_unlock(worker_stack_mutex);
    while(!Message_Is_End_Com(&port_message, 1)){
        // printf("fdd-> %d port-------------------------->> %s\n", statistics_socket, port_message.args[0]);
        previous_offset = Message_Read_from_one(&statistics_socket, 0, &port_message, buffer, previous_offset, BUFFER_DEFAULT_SIZE);
    }
    // deallocate buffer
    Message_Destroy_buffer_offset(&buffer, &previous_offset, 1);




    printf("\n\nStatistivs endedddd!! fd %d   port _> %s\n\n\n", statistics_socket, port_message.args[0]);
    Message_Delete(&port_message);
}


int queries_connection_handler(int client_socket, Port_Stack* workers_ports){
    
    // listen query from client
    Buffer* client_buffer;
    int* client_previous_offset;
    Message_vector client_message;
    Message_vector query_message;
    Message_vector results_message;
    Message_Init(&client_message);
    Message_Init(&query_message);
    Message_Init(&results_message);
    // allocate buffer for the message 
    Message_Create_buffer_offset(&client_buffer, &client_previous_offset, 1);


    client_previous_offset = Message_Read_from_one(&client_socket, 0, &client_message, client_buffer, client_previous_offset, BUFFER_DEFAULT_SIZE);
    Message_Copy(&query_message, &client_message, 0, 0);
    Message_Print(&query_message);
    while(!Message_Is_End_Com(&client_message, 1)){
        client_previous_offset = Message_Read_from_one(&client_socket, 0, &client_message, client_buffer, client_previous_offset, BUFFER_DEFAULT_SIZE);
    }


    // connect with worker to send the request
    send_workers_query_get_results(&query_message, &results_message, workers_ports);



    // deallocate buffer
    Message_Destroy_buffer_offset(&client_buffer, &client_previous_offset, 1);


    // if ( shutdown(queries_socket, SHUT_RDWR) < 0)
    //     perror("Shutdown failed");
    printf("\n\nQueries endedddd!!\n");
    Message_Delete(&client_message);
    Message_Delete(&query_message);
    Message_Delete(&results_message);
}

int send_workers_query_get_results(Message_vector* query_message , Message_vector* results_message,  Port_Stack* workers_ports){

    Buffer* worker_buffer;
    int* worker_previous_offset;

    int* port = PSTACK_Get_Stack(workers_ports);
    int port_i = 0;
    // allocate all the workers fds
    int* worker_socket = malloc(workers_ports->num_of_ports*sizeof(int));
    memset(worker_socket, 0 , workers_ports->num_of_ports*sizeof(int));

    // get all the workers and initialize the connection
    while (port[port_i] != 0){
        printf("portsss-----------------> %d\n", port[port_i]);

        worker_socket[port_i] = SOCKET_Connect(AF_INET, SOCK_STREAM, port[port_i], workers_ports->ip_address);
        check(worker_socket[port_i], "Connection error with worker");

        // set socket non-block
        check( fcntl(worker_socket[port_i], F_SETFL, fcntl(worker_socket[port_i], F_GETFL, 0) | O_NONBLOCK), "Socket server to worker Non-block error");
        // // send the command
        // Message_Write(worker_socket[port_i], query_message, BUFFER_DEFAULT_SIZE);
        port_i++;
    }  

    // send first to all the request and then wait for the reasults
    // wait and get resutls
    // all done in queries_server
    // also create the results message to pass to client
    int success = 0;
    int fail = 0;
    queries_server(query_message,  workers_ports->num_of_ports, worker_socket, worker_socket, BUFFER_DEFAULT_SIZE, &success, &fail);



    // Message_Create_buffer_offset(&worker_buffer, &worker_previous_offset, 1);

    // worker_previous_offset = Message_Read_from_one(&worker_socket, 0, results_message, worker_buffer, worker_previous_offset, BUFFER_DEFAULT_SIZE);
    // Message_Print(results_message);
    // while(!Message_Is_End_Com(results_message, 1)){
    //     worker_previous_offset = Message_Read_from_one(&worker_socket, 0, results_message, worker_buffer, worker_previous_offset, BUFFER_DEFAULT_SIZE);
    // }

    // Message_Destroy_buffer_offset(&worker_buffer, &worker_previous_offset, 1);

    // close sockets
    for (int i = 0; i <workers_ports->num_of_ports; i++){
        close(worker_socket[i]);
    }
    free(worker_socket);
    

}