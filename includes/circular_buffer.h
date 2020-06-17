#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#define STAT_TYPE 1
#define QERY_TYPE 2

#define FULL 0
#define EMPTY 1
#define AVAILABLE 2

#include <pthread.h>
#include <sys/types.h>
#include <signal.h>


typedef struct Circular_Buffer_node{

    char ip_address[32];
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

// signal flag
extern volatile sig_atomic_t signal_occured;


void CBUFFER_Init(Circular_Buffer* cbuffer, int cbuffer_size);
// for synced producer consumer buffer
void CBUFFER_Init_sync(Circular_Buffer* cbuffer, int cbuffer_size, pthread_mutex_t* cbuffer_mutex, pthread_cond_t* cbuffer_empty_condition, pthread_cond_t* cbuffer_full_condition);
void CBUFFER_Destroy(Circular_Buffer* cbuffer);

// check buffer if full
int CBUFFER_Is_Full(Circular_Buffer* cbuffer);
int CBUFFER_Is_Empty(Circular_Buffer* cbuffer);
int CBUFFER_Is_Available(Circular_Buffer* cbuffer);

// add pop
int CBUFFER_Add(Circular_Buffer* cbuffer, int socket, int type, char* ip_address);
int CBUFFER_Pop(Circular_Buffer* cbuffer, int* socket, int* type, char* ip_address);

// for synced producer consumer buffer add pop
int CBUFFER_Add_sync(Circular_Buffer* cbuffer, int socket, int type, char* ip_address);
int CBUFFER_Pop_sync(Circular_Buffer* cbuffer, int* socket, int* type, char* ip_address);

// print for debugging
void CBUFFER_Print(Circular_Buffer* cbuffer);




typedef struct Workers_Info{

    char* ip_address;
    int size;
    int num_of_workers;
    int* port; 
    int* id;

    // mutex for syncing insert and get
    pthread_mutex_t* worker_info_mutex;

} Workers_Info;



void WINFO_Init(Workers_Info* stack);
// Init with mutex
void WINFO_Init_sync(Workers_Info* stack, pthread_mutex_t* worker_info_mutex);

void WINFO_Destroy(Workers_Info* stack);

// insert synced
int WINFO_Insert_sync(Workers_Info* stack, int port, char* ip_address, int id);
int WINFO_Insert(Workers_Info* stack, int port, char* ip_address, int id);

// get copy of ports synced
int WINFO_Get_Ports_Copy_sync(Workers_Info* stack, int** copy_);
int* WINFO_Get_Stack(Workers_Info* stack);

// get num of workers synced
int WINFO_Get_num_of_workers_sync(Workers_Info* stack, int* num_of_workers);



#endif