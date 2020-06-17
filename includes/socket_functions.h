
#ifndef SOCKET_FUNCTIONS_H
#define SOCKET_FUNCTIONS_H

#include <errno.h>

#define LISTEN_DEFAULT 100000

#define check(error, string) if(error<0) { perror(string);  exit(error); }


int SOCKET_Create(int _domain, int _type, int _port, char* _addr, int _n_listen);

int SOCKET_Connect(int _domain, int _type, int _port, char* _addr);


#endif