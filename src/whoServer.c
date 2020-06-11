#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

// sockets
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

// threads
#include <pthread.h>

#include "../includes/whoServer.h"
#include "../includes/socket_functions.h"
#include "../includes/circular_buffer.h"
#include "../includes/whoServer_thread_functions.h"
#include "../includes/master_functions.h"
#include "../includes/message_handlers.h"



// mutexes
pthread_mutex_t cbuffer_mutex;
pthread_cond_t cbuffer_empty_condition;
pthread_cond_t cbuffer_full_condition;

int main(int argc, char **argv) {

    int queries_port_num = 0;
    int statistics_port_num = 0;
    int buffer_size = 0;
    int num_of_threads = 0;
    int err = 0;

    for (int i = 1; i < argc; i++){

        if(strcmp(argv[i],"-w") == 0)
            num_of_threads = atoi(argv[i+1]);
            
        else if(strcmp(argv[i],"-b") == 0)
            buffer_size = atoi(argv[i+1]);

        else if(strcmp(argv[i],"-q") == 0)
            queries_port_num = atoi(argv[i+1]);

        else if(strcmp(argv[i],"-s") == 0)
            statistics_port_num = atoi(argv[i+1]);
        
    }

    printf("threads %d  bufsize %d  stat_port %d  query_port %d\n\n", num_of_threads, buffer_size, statistics_port_num, queries_port_num);


    // init statistics socket
    int statistics_socket = SOCKET_Create(AF_INET, SOCK_STREAM, statistics_port_num, IP_ADDRESS, 500);
    check(statistics_socket, "Socket problem");
    printf("socket-> %d\n",statistics_socket);

    // init queries socket
    int queries_socket = SOCKET_Create(AF_INET, SOCK_STREAM, queries_port_num, IP_ADDRESS, 500);
    check(queries_socket, "Socket problem");
    printf("socket-> %d\n",queries_socket);


    struct sockaddr_in connection_addres_;
    memset(&connection_addres_, 0, sizeof(connection_addres_));
    socklen_t connection_addres_len_ = sizeof(connection_addres_);
    check( getsockname(statistics_socket, (struct sockaddr*) &connection_addres_, &connection_addres_len_), "Socket get name error" );
    uint16_t queries_socket_port_ = ntohs(connection_addres_.sin_port);
    printf("\n\n\n port numberr %d \n\n\n", ntohs(connection_addres_.sin_port));


    // thread mutexes
    pthread_mutex_init(&cbuffer_mutex, NULL);
    pthread_cond_init(&cbuffer_empty_condition, NULL);
    pthread_cond_init(&cbuffer_full_condition, NULL);

    // Circular buffer init synced
    Circular_Buffer circular_buffer;
    CBUFFER_Init_sync(&circular_buffer, buffer_size, &cbuffer_mutex, &cbuffer_empty_condition, &cbuffer_full_condition);


    // init threads
    pthread_t* thread = malloc(num_of_threads*sizeof(pthread_t));
    memset(thread, 0, num_of_threads*sizeof(pthread_t));
    
    // thread arguments
    Thread_Args thread_args; 
    thread_args.circular_buffer = &circular_buffer;
    thread_args.counter = 0;


    
    for (int i = 0; i < num_of_threads; i++){

        err = pthread_create( &(thread[i]), NULL, thread_connection_handler, (void*) &thread_args);
        check_t(err, "Thread Create error");

    }
    



    // struct sockaddr *client;
    struct sockaddr_in connection_addres;
    socklen_t connection_addres_len = 0;
    int new_sock;
    FILE* sock_stream;
    char* buf[100];

    // init fd set for select for 2 sockets
    fd_set set, read_set;
    FD_ZERO(&set);
    FD_SET(statistics_socket, &set);
    FD_SET(queries_socket, &set);
    int max_fd = statistics_socket > queries_socket ? statistics_socket : queries_socket;
    max_fd++;
    int activity = 0;
    printf("MAAAAAAAAAX  %d\n\n",max_fd);

    while(1){
        read_set = set;

        // select activity over all 2 availabel sockets of queries and statistics
        activity = select(max_fd, &read_set, NULL, NULL, NULL);

        if( activity < 0)
            perror("Select error:");
        else{
            printf("counter test0>>> %d <<<<<<<<\n\n",thread_args.counter);
            // check if statistics has something
            if ( FD_ISSET(statistics_socket, &read_set) ){

                printf("statosotoooooooooooooooooocssssssss\n");
                new_sock = accept(statistics_socket, (struct sockaddr*) &connection_addres, &connection_addres_len);
                check(new_sock, "Accept failed");
                // if (() < 0){
                //     perror("Accept failed");
                //     exit(new_sock);
                // }
                // pthread_mutex_lock(&cbuffer_mutex);

                // while( CBUFFER_Is_Full(&circular_buffer)){
                //     printf("found buffer full!! \n\n");
                //     pthread_cond_wait(&cbuffer_empty_condition, &cbuffer_mutex);
                // }

                // CBUFFER_Add(&circular_buffer, new_sock, STAT_TYPE);
                
                // pthread_mutex_unlock(&cbuffer_mutex);
                // pthread_cond_signal(&cbuffer_full_condition);

                CBUFFER_Add_sync(&circular_buffer, new_sock, STAT_TYPE);


                printf("New connection!!\n");
                // sock_stream = fdopen(new_sock, "r+");
                // // listen to client
                // fgets(buf, 100, sock_stream);
                // printf("%s\n",buf);
                // if ( shutdown(new_sock, SHUT_RDWR) < 0)
                //     perror("Shutdown failed");

            }

            // check if queries has something
            if ( FD_ISSET(queries_socket, &read_set) ){

                printf("queeeeeeeeeeriiiiiiisssssssssssss\n");
                new_sock = accept(queries_socket, (struct sockaddr*) &connection_addres, &connection_addres_len);
                check(new_sock, "Accept failed");

                CBUFFER_Add_sync(&circular_buffer, new_sock, QERY_TYPE);

                printf("New connection!!\n");
                

            }
            CBUFFER_Print(&circular_buffer);
            
        }

    }

    // destroy c buffer
    CBUFFER_Destroy(&circular_buffer);

    // destroy mutexes
    check_t( pthread_mutex_destroy(&cbuffer_mutex), "Thread mutex destroy error");
    check_t( pthread_cond_destroy(&cbuffer_empty_condition), "Thread condition destroy error");
    check_t( pthread_cond_destroy(&cbuffer_full_condition), "Thread condition destroy error");

    // wait for threads to join
    for (int i = 0; i < num_of_threads; i++){

        err = pthread_join(thread[i], NULL);
        // check_t(err, "Thread Join error");

    }
    free(thread);

}

 // uint32_t a = 0xAABBCCDD;
    // a = htonl(a);
    // char *ap = &a;
    // printf("hello wolrd  %x\n \n\n %d \n\n", *(ap + 1), getpid());

    // struct hostent *host;

    // if ((host = gethostbyname(argv[1])) == NULL){
    //     printf("la8osss\n\n");
    // }

    // printf("name-> %s, %d\n", host->h_name, host->h_length);

    // char *addr[64];
    // struct   in_addr  ** addr_list;
    // addr_list = (struct in_addr**) host->h_addr_list;

    // for (int i = 0; addr_list[i] != NULL ; i++){
    //     strcpy(addr, inet_ntoa( addr_list[i][0]) );
    //     printf("adr-> %s\n",addr);
    // }



    // struct in_addr myaddr;
    // inet_aton(addr_list[0], &myaddr);

    // if (      (  host = gethostbyaddr( &myaddr, sizeof(myaddr), AF_INET) ) == NULL          ){
    //     printf("la8osss\n\n");
    // }

    // printf("ip name-> %s\n", host->h_name);