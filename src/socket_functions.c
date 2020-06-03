#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <netdb.h>
// sockets
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// create bind and set listening socket
// return the socket file descriptor
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
    sock_addr_info.sin_port = htons(_port);

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