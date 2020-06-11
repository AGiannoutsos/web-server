#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../includes/whoClient.h"
#include "../includes/whoClient_thread_functions.h"
#include "../includes/socket_functions.h"
#include "../includes/queries_list.h"


int main(int argc, char** argv){
    printf("hello wolrd\n");

    char* queries_file_name;
    char* server_ip;
    int server_port = 0;
    int num_of_threads = 0;
    int err = 0;

    for (int i = 1; i < argc; i++){

        if(strcmp(argv[i],"-w") == 0)
            num_of_threads = atoi(argv[i+1]);
            
        else if(strcmp(argv[i],"-q") == 0)
            queries_file_name = argv[i+1];

        else if(strcmp(argv[i],"-sip") == 0)
            server_ip = argv[i+1];

        else if(strcmp(argv[i],"-sp") == 0)
            server_port = atoi(argv[i+1]);
        
    }

    printf("threads %d  query_file %s  server_port %d  server_ip %s\n\n", num_of_threads, queries_file_name, server_port, server_ip);

    // init queries list in order threads have a unified place where queries are stored
    Queries_List queries_list;
    QLIST_Init(&queries_list);

    // init threads
    pthread_t* thread = malloc(num_of_threads*sizeof(pthread_t));
    memset(thread, 0, num_of_threads*sizeof(pthread_t));
    int thread_i = 0;

    // open queries file
    FILE* queries_file_stream = fopen(queries_file_name, "r");
    if (queries_file_stream == NULL ){ perror("File problem"); exit(1);}




    char read_file_buffer[READ_FILE_BUFFER] = {0};
    int num_of_queries_read = 0;


    // read every query in file
    fgets(read_file_buffer, READ_FILE_BUFFER, queries_file_stream); 
    while( !feof(queries_file_stream)  &&  read_file_buffer[0] != '\n'){
        num_of_queries_read++;

        // insert to queries list
        QLIST_Insert(&queries_list, read_file_buffer);
        printf("%d> %s",  num_of_queries_read, QLIST_Get_Last_query(&queries_list));

        // create thread and pass it the query
        err = pthread_create( &(thread[thread_i]), NULL, thread_connection_handler, (void*) QLIST_Get_Last_query(&queries_list));
        check_t(err, "Thread Create error");
        thread_i++;

        // get next query
        memset(read_file_buffer, 0, READ_FILE_BUFFER);
        fgets(read_file_buffer, READ_FILE_BUFFER, queries_file_stream);

        // if num of queries have reached the maximun number of threads then its time to send the data to server all together
        // or query file has reached its end
        if( (num_of_queries_read % num_of_threads == 0) || feof(queries_file_stream) || read_file_buffer[0] == '\n'){
            printf("send queries all together\n\n");

            //!!/ muted down start_query

            // mutex down new_thread

            // while( threads_online < threads_i){
            //      wait_signa new_thread()
            //mutex up new_thread

            // mutex down start_query
            // thread_id_x = x
            // mutex_up start_query
            // bradcast_ start_query  



            // wait for threads to join
            for (int i = 0; i < thread_i; i++){
                err = pthread_join(thread[i], NULL);
                perror_t(err, "Thread Join error");
                // pritf("\n\n-->>%s<<---\n\n", strerror(err));
                printf("joiedd\n");
            }
            thread_i = 0;
        }

    }


    // create socket
    // int sock = socket(AF_INET, SOCK_STREAM, 0);
    // if (sock == -1) perror("Socket creation");

    // // sock addr
    // struct in_addr addr;

    // struct sockaddr_in server;
    // server.sin_family = AF_INET;
    // inet_aton("0.0.0.0",&addr);
    // // memcpy()
    // server.sin_addr.s_addr = addr.s_addr;
    // server.sin_port = htons(atoi(argv[1]));
    // // server.sin_port = htons(10000);


    // // connect to server
    // char buf[100];
    // if ( connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0 )
    //     perror("Connection failed");
    // printf("Connected!!");
    sleep(1);
    fclose(queries_file_stream);
    QLIST_Destroy(&queries_list);
    free(thread);

    char buf[100] = {0};

    int sock = SOCKET_Connect(AF_INET, SOCK_STREAM, server_port, server_ip);
    check(sock, "Connection failed");

    FILE* sock_sream = fdopen(sock, "r+");

    // while(strcmp(buf, "exit") != 0){
        fgets(buf, 100, stdin);
        fprintf(sock_sream, "%s xixi\n", buf);

    // }


}