#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#define STAT_TYPE 1
#define QERY_TYPE 2

#define FULL 0
#define EMPTY 1
#define AVAILABLE 2

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

} Circular_Buffer;


void CBUFFER_Init(Circular_Buffer* cbuffer, int cbuffer_size);
void CBUFFER_Destroy(Circular_Buffer* cbuffer);

// check buffer if empty
int CBUFFER_Is_Full(Circular_Buffer* cbuffer);
int CBUFFER_Is_Empty(Circular_Buffer* cbuffer);
int CBUFFER_Is_Available(Circular_Buffer* cbuffer);

// add pop
int CBUFFER_Add(Circular_Buffer* cbuffer, int socket, int type);
int CBUFFER_Pop(Circular_Buffer* cbuffer, int* socket, int* type);

// print for debugging
void CBUFFER_Print(Circular_Buffer* cbuffer);

#endif