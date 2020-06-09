#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#define STAT_TYPE 1
#define QERY_TYPE 2

#define FULL 0
#define EMPTY 1
#define AVAILABLE 2

#include <pthread.h>


typedef struct Circular_Buffer_node{

    int socket;
    int type;

} Circular_Buffer_node;

typedef struct Circular_Buffer{

    int is_state;
    int size;
    int head;
    int tail;
    Circular_Buffer_node* cbuffer_node;

    // sync mutexes
    pthread_mutex_t* cbuffer_mutex;
    pthread_cond_t* cbuffer_empty_condition;
    pthread_cond_t* cbuffer_full_condition;

} Circular_Buffer;


void CBUFFER_Init(Circular_Buffer* cbuffer, int cbuffer_size);
// for synced producer consumer buffer
void CBUFFER_Init_sync(Circular_Buffer* cbuffer, int cbuffer_size, pthread_mutex_t* cbuffer_mutex, pthread_cond_t* cbuffer_empty_condition, pthread_cond_t* cbuffer_full_condition);
void CBUFFER_Destroy(Circular_Buffer* cbuffer);

// check buffer if full
int CBUFFER_Is_Full(Circular_Buffer* cbuffer);
int CBUFFER_Is_Empty(Circular_Buffer* cbuffer);
int CBUFFER_Is_Available(Circular_Buffer* cbuffer);

// add pop
int CBUFFER_Add(Circular_Buffer* cbuffer, int socket, int type);
int CBUFFER_Pop(Circular_Buffer* cbuffer, int* socket, int* type);

// for synced producer consumer buffer add pop
int CBUFFER_Add_sync(Circular_Buffer* cbuffer, int socket, int type);
int CBUFFER_Pop_sync(Circular_Buffer* cbuffer, int* socket, int* type);

// print for debugging
void CBUFFER_Print(Circular_Buffer* cbuffer);

#endif