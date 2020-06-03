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

#include "../includes/whoClient_thread_functions.h"

int main(int argc, char** argv){
    printf("hello wolrd\n");


    // create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) perror("Socket creation");

    // sock addr
    struct in_addr addr;

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    inet_aton("0.0.0.0",&addr);
    // memcpy()
    server.sin_addr.s_addr = addr.s_addr;
    server.sin_port = htons(atoi(argv[1]));
    // server.sin_port = htons(10000);


    // connect to server
    char buf[100];
    if ( connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0 )
        perror("Connection failed");
    printf("Connected!!");

    FILE* sock_sream = fdopen(sock, "r+");

    // while(strcmp(buf, "exit") != 0){
        fgets(buf, 100, stdin);
        fprintf(sock_sream, "%s xixi\n", buf);

    // }


}