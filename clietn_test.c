
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>

// sockets
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

// #include "./includes/socket_functions.h"

int SOCKET_Create(int _domain, int _type, int _port, char* _addr, int _n_listen){
    int err = 0;

    // create socket
    int sock = socket(_domain, _type, 0);
    if (sock < 0){
        perror("Socket creation");
        return sock;
    }

    // re use socket
    int opt_val = 1;
    err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(int));
    if ( err < 0 ){
        perror("Sock reuse option failed");
        return err;
    }

    // sock addr
    struct sockaddr_in sock_addr_info;
    struct in_addr addr;
    inet_aton(_addr, &addr);
    sock_addr_info.sin_family = _domain;
    sock_addr_info.sin_addr.s_addr = addr.s_addr; 
    sock_addr_info.sin_port = htons( (uint16_t)_port );

    // bind
    err = bind(sock, (struct sockaddr*) &sock_addr_info, sizeof(sock_addr_info)); 
    if ( err < 0 ){
        perror("Bind failed");
        return err;
    }

    // listen 
    err = listen(sock, _n_listen);
    if ( err < 0 ){
        perror("Listen failed");
        return err;
    }

    // return socket fd
    return sock;

}

// connect to a socket and return thee file descriptor
int SOCKET_Connect(int _domain, int _type, int _port, char* _addr){
    int err = 0;

    // create socket
    int sock = socket(_domain, _type, 0);
    if (sock < 0){
        perror("Socket creation");
        return sock;
    }

    // sock addr
    struct sockaddr_in sock_addr_info;
    struct in_addr addr;
    inet_aton(_addr, &addr);
    sock_addr_info.sin_family = _domain;
    sock_addr_info.sin_addr.s_addr = addr.s_addr; 
    sock_addr_info.sin_port = htons( (uint16_t)_port );



    // connect to server
    err = connect(sock, (struct sockaddr*) &sock_addr_info, sizeof(sock_addr_info));
    if ( err < 0 ){
        perror("Connection failed");
        return err;
    }

    return sock;
}

// close socket safely
int SOCKET_Close(int socket){

    shutdown(socket, SHUT_RDWR);

    char buffer[1];
    int bytes_read = 0;
    // while( (bytes_read = read(socket, buffer, 1)) == 0 ){
    //     printf("bytes read -> %d",bytes_read);
    // }
    // printf("bytes read ---> %d",bytes_read);

    close(socket);
}

#define size 10000

int main(int argd, char** argv){


    int sock = SOCKET_Connect(AF_INET, SOCK_STREAM, 10000, "0.0.0.0");

    // int sock = socket(AF_INET, SOCK_STREAM, 0);  
    // connect(sock, &remote, sizeof(remote));
    // sleep(5);
    int mb = atoi(argv[1]);
    char buffer[size];
    memset(buffer, 13, size);
    for (int i = 0; i < mb; i++){
        write(sock, buffer, size);             // returns 1000000
        printf("sent %d mb\n", i); fflush(stdout);
    }
    // sleep(1);
    sleep(2);
    // shutdown(sock, SHUT_RDWR);
    // printf("SHUTTTDOWNN\n");
    // sleep(2);
    // printf("CLODEDDDDDDDD\n");
    SOCKET_Close(sock);
}





