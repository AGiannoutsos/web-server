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

// mutexes for synchronization
// start thread mutexes
pthread_mutex_t threads_online_mutex;
pthread_cond_t threads_online_condition;

pthread_mutex_t start_queries_mutex;
pthread_cond_t start_queries_condition;

int start_queries;
int threads_online;

// end thread mutexes
pthread_mutex_t threads_offline_mutex;
pthread_cond_t threads_offline_condition;

pthread_mutex_t end_queries_mutex;
pthread_cond_t end_queries_condition;

int end_queries;
int threads_offline;

// mutexes for printing the results
pthread_mutex_t print_mutex;




int main(int argc, char** argv){


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

    // init mutexes
    // and shared variables
    start_queries = 0;
    threads_online = 0;
    pthread_mutex_init(&threads_online_mutex, NULL);
    pthread_cond_init(&threads_online_condition, NULL);

    pthread_mutex_init(&start_queries_mutex, NULL);
    pthread_cond_init(&start_queries_condition, NULL);

    end_queries = 0;
    threads_offline = 0;
    pthread_mutex_init(&threads_offline_mutex, NULL);
    pthread_cond_init(&threads_offline_condition, NULL);

    pthread_mutex_init(&end_queries_mutex, NULL);
    pthread_cond_init(&end_queries_condition, NULL);

    pthread_mutex_init(&print_mutex, NULL);


    // init threads
    pthread_t* thread = malloc(num_of_threads*sizeof(pthread_t));
    memset(thread, 0, num_of_threads*sizeof(pthread_t));
    int thread_i = 0;
    // init thread arhuments
    Thread_Args* thread_args = malloc(num_of_threads*sizeof(Thread_Args));
    memset(thread_args, 0, num_of_threads*sizeof(Thread_Args));

    // init attribute
    pthread_attr_t attribute;
    pthread_attr_init(&attribute);
    pthread_attr_setdetachstate(&attribute, 1);

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
        // printf("%d> %s",  num_of_queries_read, QLIST_Get_Last_query(&queries_list));
        // create the thread args
        thread_args[thread_i].query = QLIST_Get_Last_query(&queries_list);
        thread_args[thread_i].server_ip = server_ip;
        thread_args[thread_i].server_port = server_port;
        thread_args[thread_i].print_mutex = &print_mutex;

        // create thread and pass it the query
        err = pthread_create( &thread[thread_i], NULL, thread_connection_handler, (void*) &(thread_args[thread_i]));
        check_t(err, "Thread Create error");
        thread_i++;

        // get next query
        memset(read_file_buffer, 0, READ_FILE_BUFFER);
        fgets(read_file_buffer, READ_FILE_BUFFER, queries_file_stream);

        // if num of queries have reached the maximun number of threads then its time to send the data to server all together
        // or query file has reached its end
        if( (num_of_queries_read % num_of_threads == 0) || feof(queries_file_stream) || read_file_buffer[0] == '\n'){
            // printf("send queries all together\n\n");

            //!!/ muted down start_query

            // mutex down new_thread

            // while( threads_online < threads_i){
            //      wait_signa new_thread()
            //mutex up new_thread

            // mutex down start_query
            // thread_id_x = x
            // mutex_up start_query
            // bradcast_ start_query  

            // wait for threads to start at the same time
            pthread_mutex_lock(&threads_online_mutex);
            while( threads_online < thread_i){
                // printf(" online %d i %d\n",threads_online, thread_i);
                pthread_cond_wait(&threads_online_condition, &threads_online_mutex);
            }
            pthread_mutex_unlock(&threads_online_mutex);

            end_queries = 0;
            threads_offline = 0;

            pthread_mutex_lock(&start_queries_mutex);
            start_queries = 1;
            pthread_mutex_unlock(&start_queries_mutex);
            pthread_cond_broadcast(&start_queries_condition);

            


                // sleep(1);

            // wait for all the threads to end at the same time
            pthread_mutex_lock(&threads_offline_mutex);
            printf(" offline %d i %d\n",threads_offline, thread_i);
            while( threads_offline < thread_i){
                printf(" offline %d i %d\n",threads_offline, thread_i);
                pthread_cond_wait(&threads_offline_condition, &threads_offline_mutex);
            }
            pthread_mutex_unlock(&threads_offline_mutex);

            start_queries = 0;
            threads_online = 0;

            pthread_mutex_lock(&end_queries_mutex);
            end_queries = 1;
            pthread_mutex_unlock(&end_queries_mutex);
            pthread_cond_broadcast(&end_queries_condition);


            for (int i = 0; i < thread_i; i++){
                err = pthread_join(thread[i], NULL);
                if (err < 0 ){
                    perror("Join thread error\n");
                }
            }

            // reset variables after all threads have ended but not queries have ended 
            if ( !(feof(queries_file_stream) || read_file_buffer[0] == '\n') ){
                memset(thread, 0, num_of_threads*sizeof(pthread_t));
                thread_i = 0;
            }
            // wait for threads to join

            // sleep(4);

        }

    }

    // for (int i = 0; i < thread_i; i++){
    //     printf("joiedd %d\n",thread[i]);
    //     err = pthread_join(thread[i], NULL);
    //     // check_t(err, "Thread Join error");
    //     // pritf("\n\n-->>%s<<---\n\n", strerror(err));
    //     perror("IIIIIIIIIIIIIIIIII\n");
    // }

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
    // sleep(1);

    // check_t( pthread_mutex_destroy(&threads_online_mutex), "Thread mutex destroy error");
    // check_t( pthread_mutex_destroy(&threads_offline_mutex), "Thread mutex destroy error");
    // check_t( pthread_mutex_destroy(&start_queries_mutex), "Thread mutex destroy error");
    // check_t( pthread_mutex_destroy(&end_queries_mutex), "Thread mutex destroy error");

    // check_t( pthread_cond_destroy(&threads_online_condition), "Thread condition destroy error");
    // check_t( pthread_cond_destroy(&threads_offline_condition), "Thread condition destroy error");
    // check_t( pthread_cond_destroy(&start_queries_condition), "Thread condition destroy error");
    // check_t( pthread_cond_destroy(&end_queries_condition), "Thread condition destroy error");
    // sleep(1);
    fclose(queries_file_stream);
    QLIST_Destroy(&queries_list);
    free(thread);
    free(thread_args);
    if( err = pthread_detach(pthread_self()) ) {
        perror_t("Thread Detach error ", err);
        exit(1);
    }
    // pthread_exit(NULL); 
    return 0; 
    // pthread_exit(NULL);
    // char buf[100] = {0};

    // int sock = SOCKET_Connect(AF_INET, SOCK_STREAM, server_port, server_ip);
    // check(sock, "Connection failed");

    // FILE* sock_sream = fdopen(sock, "r+");

    // // while(strcmp(buf, "exit") != 0){
    //     fgets(buf, 100, stdin);
    //     fprintf(sock_sream, "%s xixi\n", buf);

    // // }


}