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



int counter = 0;

void *thread_connection_handler(void *args){
    int err = 0;
    Thread_Args* thread_args = (Thread_Args*)args;
    Circular_Buffer* circular_buffer = thread_args->circular_buffer;
    Workers_Info* workers_info = thread_args->workers_info;
    pthread_mutex_t* print_mutex = thread_args->print_mutex;
    pthread_mutex_t* query_mutex = thread_args->query_mutex;

    

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
    char ip_address[32];
    while( 1 ){

        // pop from buffer with mutex safety synv
        CBUFFER_Pop_sync(circular_buffer, &socket, &type, ip_address);

        // printf("poped %d %d  %s     %d \n", socket, type, ip_address, CBUFFER_Is_Empty(circular_buffer));
        // sleep(1);
        // exit if signal
        if (signal_occured == 1)
            break;

        

        // gather indormation about worker and statistics
        if (type == STAT_TYPE){
            // printf("statistics handdler\n");
            statistics_connection_handler(socket, ip_address, workers_info, print_mutex);
        }
        else if (type == QERY_TYPE){
            // printf("xixi query handler\n");
            queries_connection_handler(socket, workers_info, print_mutex, query_mutex);
            // pthread_mutex_lock(print_mutex);
            // counter++;
            // fprintf(stderr, "counter----<><><><>>>>>    %d  %d\n", counter, socket);
            // pthread_mutex_unlock(print_mutex);
        }

        // close socket after use
        // sleep(1);
            // pthread_mutex_lock(query_mutex); 

        close(socket); 
            // pthread_mutex_unlock(query_mutex); 

        // fprintf(stderr, "c %d\n", socket);
    }


    // exitng thred detaching
    // if( err = pthread_detach(pthread_self()) ) {
    //     perror_t("Thread Ditach error ", err);
    //     exit(1);
    // }

    // exiting
    // pthread_exit(NULL);
}

int statistics_connection_handler(int statistics_socket, char* ip_address, Workers_Info* workers_info, pthread_mutex_t* print_mutex){

    // set socket NONBLOCK
    //  int err = fcntl(statistics_socket, F_SETFL, fcntl(statistics_socket, F_GETFL, 0) | O_NONBLOCK);
    //     check(err, "Socket Non-block error");
    int* previous_offset;
    Buffer* buffer;
    Message_vector port_message;
    Message_Init(&port_message);
    // allocate buffer for the message 
    Message_Create_buffer_offset(&buffer, &previous_offset, 1);

    // read_statistics_from_workers(0, 1, &statistics_socket, buffer, previous_offset, BUFFER_DEFAULT_SIZE);
    Message_vector statistics;
    Message_Init(&statistics);

    // read statistics and print them safely with print mutex
    previous_offset = Message_Read_from_one_socket(&statistics_socket, 0, &statistics, buffer, previous_offset, BUFFER_DEFAULT_SIZE);
    while(!Message_Is_End_Com(&statistics, 1)){
        // lock print mutex
        pthread_mutex_lock(print_mutex);
        Statistics_Print(&statistics, stderr);
        pthread_mutex_unlock(print_mutex);

        previous_offset = Message_Read_from_one_socket(&statistics_socket, 0, &statistics, buffer, previous_offset, BUFFER_DEFAULT_SIZE);
    }

    Message_Delete(&statistics);

    // read worker socket port
    previous_offset = Message_Read_from_one(&statistics_socket, 0, &port_message, buffer, previous_offset, BUFFER_DEFAULT_SIZE);
    // store port of worker in a data structure
    // use sync with worker info mutex installed in it
    // when inserting id must be unique
    // other wise it will overight the port of the existing id in case that a worker dies and resends its port number
    WINFO_Insert_sync(workers_info, atoi( port_message.args[0]), ip_address, atoi(port_message.args[2]) );
    // int* a = WINFO_Get_Stack(workers_info);
    // int k = 0;
    // while (a[k] != -1){
    //     printf("portsss-----------------> %d  %s  %d\n", a[k], workers_info->ip_address, workers_info->id[k]);
    //     k++;
    // }  


    while(!Message_Is_End_Com(&port_message, 1)){
        // printf("fdd-> %d port-------------------------->> %s\n", statistics_socket, port_message.args[0]);
        previous_offset = Message_Read_from_one(&statistics_socket, 0, &port_message, buffer, previous_offset, BUFFER_DEFAULT_SIZE);
    }
    // deallocate buffer
    Message_Destroy_buffer_offset(&buffer, &previous_offset, 1);



    // close(statistics_socket);
    // printf("\n\nStatistivs endedddd!! fd %d   port _> %s\n\n\n", statistics_socket, port_message.args[0]);
    Message_Delete(&port_message);
}


int queries_connection_handler(int client_socket, Workers_Info* workers_info, pthread_mutex_t* print_mutex, pthread_mutex_t* query_mutex){
    
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

    // get request from client
        // pthread_mutex_lock(print_mutex);
    // printf("1  %d\n", client_socket);
    // usleep(100000);
    client_previous_offset = Message_Read_from_one_socket(&client_socket, 0, &client_message, client_buffer, client_previous_offset, 1000);
    // printf("2\n");
    Message_Copy(&query_message, &client_message, 0, 0);
    // printf("3\n");
    // Message_Print(&query_message);
    while(!Message_Is_End_Com(&client_message, 1)){
        // printf("4\n");
        client_previous_offset = Message_Read_from_one_socket(&client_socket, 0, &client_message, client_buffer, client_previous_offset, 1000);
        // printf("5\n");
    }
        // printf("6\n");
        // pthread_mutex_unlock(print_mutex);
    // printf("7\n");

    // print query
    // pthread_mutex_lock(print_mutex);
    pthread_mutex_lock(query_mutex); 
    printf("\n");
    for (int i = 0; i < query_message.num_of_args; i++)
        printf("%s ", query_message.args[i]);
    printf("\n");
    // pthread_mutex_unlock(print_mutex);

    // sleep(1);

    // connect with workers to send the request and get the results
    send_workers_query_get_results(&query_message, &results_message, workers_info);
    pthread_mutex_unlock(query_mutex);

    // sleep(1);
    // printf("11\n");

    // send client the results
    Message_Write(client_socket, &results_message, BUFFER_DEFAULT_SIZE);
    Message_Write_End_Com(client_socket, BUFFER_DEFAULT_SIZE);

    // // wait so that clietn has received all the data then close the socket
    // client_previous_offset = Message_Read_from_one(&client_socket, 0, &client_message, client_buffer, client_previous_offset, BUFFER_DEFAULT_SIZE);
    // while(!Message_Is_End_Com(&client_message, 1)){
    //     client_previous_offset = Message_Read_from_one(&client_socket, 0, &client_message, client_buffer, client_previous_offset, BUFFER_DEFAULT_SIZE);
    // }
    // sleep(1);
    // int k = 0;
    // int a = 0;
    // char inter_buffer[100];
    // printf("read %d\n",a); fflush(stdout);
    // a = read(client_socket, inter_buffer, 10);
    // printf("read %d\n",a); fflush(stdout);
    // while(a != -2){
    //     // sleep(1);
    //     printf("read %d %d\n",a, client_socket); fflush(stdout);
    //     printf("--\n");
    //     a = read(client_socket, inter_buffer, 10);
    //     k++;
    //     if(k>4) break;
    // }


    // close(client_socket);

    //  fprintf(stderr, "c %d\n", client_socket);

    // fd_set set, read_set, write_set;
    // FD_ZERO(&set);
    // FD_SET(client_socket ,&set);
    // int a = 1;
    // int activity = 1;
    // struct timeval time;
    // struct timeval timee;
    // time.tv_sec = 1;
    // time.tv_usec = 2000000;
    // // close(client_socket);
    // while(a < 3){
    //     sleep(1);
    //     timee.tv_sec = 1;
    //     timee.tv_usec = 0;
    //     read_set = set;
    //     write_set = set;
    //     activity = select(client_socket+1, &read_set, &write_set, NULL, &timee);
    //     printf("activity %d %d\n",activity, a);
    //     a++;
    // }



    // deallocate buffer
    Message_Destroy_buffer_offset(&client_buffer, &client_previous_offset, 1);
    // if ( shutdown(client_socket, SHUT_RDWR) < 0)
    //     perror("Shutdown failed");
    // printf("\n\nQueries endedddd!!\n");
    Message_Delete(&client_message);
    Message_Delete(&query_message);
    Message_Delete(&results_message);
}


int send_workers_query_get_results(Message_vector* query_message , Message_vector* results_message,  Workers_Info* workers_info){

    Buffer* worker_buffer;
    int* worker_previous_offset;
    int port_i = 0;

    // get copy of ports and num of workers synced safely
    int num_of_workers;
    WINFO_Get_num_of_workers_sync(workers_info, &num_of_workers);
    int* port;
    WINFO_Get_Ports_Copy_sync(workers_info, &port);

    int* worker_socket = malloc(num_of_workers*sizeof(int));
    memset(worker_socket, 0 , num_of_workers*sizeof(int));
    

    // get all the workers and initialize the connection
    while (port[port_i] != -1){
        // printf("portsss-----------------> %d\n", port[port_i]);
        worker_socket[port_i] = SOCKET_Connect(AF_INET, SOCK_STREAM, port[port_i], workers_info->ip_address);
        // printf("socket fd %d\n",worker_socket[port_i]);
        // if port connaction wasnt availabe then abbort further more workers
        if (worker_socket[port_i] < 0){
            perror("Connection error with worker PORT wanst available!\nAbort query");
            exit(100);
            num_of_workers = port_i;
            break;
        }

        // set socket non-block
        // check( fcntl(worker_socket[port_i], F_SETFL, fcntl(worker_socket[port_i], F_GETFL, 0) | O_NONBLOCK), "Socket server to worker Non-block error");

        port_i++;
    }  

    // send first to all the request and then wait for the reasults
    // wait and get resutls
    // also create the results message to pass to client
    int success = 0;
    int fail = 0;
    queries_server(query_message,  results_message, num_of_workers, worker_socket, worker_socket, BUFFER_DEFAULT_SIZE, &success, &fail);



    // close sockets
    for (int i = 0; i < num_of_workers; i++){
        close(worker_socket[i]);
    }
    free(worker_socket);
    free(port);
    
}