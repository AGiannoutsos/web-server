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

    printf("threads %d  bufsize %d  stat_port %d  query_port %d\n\n", num_of_threads, buffer_size, queries_port_num, statistics_port_num);


    // init statistics socket
    int statistics_socket = SOCKET_Create(AF_INET, SOCK_STREAM, statistics_port_num, IP_ADDRESS, 500);
    if ( statistics_socket < 0){
        perror("Socket problem:");
        exit(statistics_socket);
    }
    printf("socket-> %d\n",statistics_socket);

    // init queries socket
    int queries_socket = SOCKET_Create(AF_INET, SOCK_STREAM, queries_port_num, IP_ADDRESS, 500);
    if ( queries_socket < 0){
        perror("Socket problem:");
        exit(queries_socket);
    }
    printf("socket-> %d\n",queries_socket);



    // Circular buffer init
    Circular_Buffer circular_buffer;
    CBUFFER_Init(&circular_buffer, 5);



    // init threads
    pthread_t* thread = malloc(num_of_threads*sizeof(pthread_t));
    memset(thread, 0, num_of_threads*sizeof(pthread_t));
    
    // thread arguments
    Thread_Args thread_args; 
    thread_args.circular_buffer = &circular_buffer;
    
    for (int i = 0; i < num_of_threads; i++){

        if( err = pthread_create( &(thread[i]), NULL, thread_connection_handler, (void*) &thread_args) ) {
            perror_t("Thread Create error", err);
            exit(1);
        }
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

            // check if statistics has something
            if ( FD_ISSET(statistics_socket, &read_set) ){

                printf("statosotoooooooooooooooooocssssssss\n");
                if ((new_sock = accept(statistics_socket, (struct sockaddr*) &connection_addres, &connection_addres_len)) < 0){
                    perror("Accept failed");
                    exit(new_sock);
                }
                CBUFFER_Add(&circular_buffer, new_sock, STAT_TYPE);

                printf("New connection!!\n");
                sock_stream = fdopen(new_sock, "r+");
                // listen to client
                fgets(buf, 100, sock_stream);
                printf("%s\n",buf);
                if ( shutdown(new_sock, SHUT_RDWR) < 0)
                    perror("Shutdown failed");

            }

            // check if queries has something
            if ( FD_ISSET(queries_socket, &read_set) ){

                printf("queeeeeeeeeeriiiiiiisssssssssssss\n");
                if ((new_sock = accept(queries_socket, (struct sockaddr*) &connection_addres, &connection_addres_len)) < 0){
                    perror("Accept failed");
                    exit(new_sock);
                }
                CBUFFER_Add(&circular_buffer, new_sock, QERY_TYPE);
                printf("New connection!!\n");
                sock_stream = fdopen(new_sock, "r+");
                // listen to client
                fgets(buf, 100, sock_stream);
                printf("%s\n",buf);
                if ( shutdown(new_sock, SHUT_RDWR) < 0)
                    perror("Shutdown failed");

            }
            CBUFFER_Print(&circular_buffer);
            
        }

    }

    // destroy c buffer
    CBUFFER_Destroy(&circular_buffer);

    // wait for threads to join
    for (int i = 0; i < num_of_threads; i++){

        if( err = pthread_join(thread[i], NULL) ) {
            perror_t("Thread Join error", err);
            exit(1); 
        }

    }

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