#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>


#include "../includes/circular_buffer.h"


// init circular buffer
void CBUFFER_Init(Circular_Buffer* cbuffer, int cbuffer_size){

    memset(cbuffer, 0, sizeof(Circular_Buffer));
    cbuffer->size = cbuffer_size;
    cbuffer->head = 0;
    cbuffer->tail = 0;
    cbuffer->is_state = EMPTY;

    // non sync buffer
    cbuffer->cbuffer_mutex = NULL;
    cbuffer->cbuffer_empty_condition = NULL;
    cbuffer->cbuffer_full_condition = NULL;


    cbuffer->cbuffer_node = malloc(cbuffer_size*sizeof(Circular_Buffer_node));
    for (int i = 0; i < cbuffer_size; i++){
        cbuffer->cbuffer_node[i].socket = 0;
        cbuffer->cbuffer_node[i].type = 0;
    }

    
}

void CBUFFER_Init_sync(Circular_Buffer* cbuffer, int cbuffer_size, pthread_mutex_t* cbuffer_mutex, pthread_cond_t* cbuffer_empty_condition, pthread_cond_t* cbuffer_full_condition){

    memset(cbuffer, 0, sizeof(Circular_Buffer));
    cbuffer->size = cbuffer_size;
    cbuffer->head = 0;
    cbuffer->tail = 0;
    cbuffer->is_state = EMPTY;

    // sync buffer
    cbuffer->cbuffer_mutex = cbuffer_mutex;
    cbuffer->cbuffer_empty_condition = cbuffer_empty_condition;
    cbuffer->cbuffer_full_condition = cbuffer_full_condition;

    cbuffer->cbuffer_node = malloc(cbuffer_size*sizeof(Circular_Buffer_node));
    for (int i = 0; i < cbuffer_size; i++){
        cbuffer->cbuffer_node[i].socket = 0;
        cbuffer->cbuffer_node[i].type = 0;
    }
    
}

void CBUFFER_Destroy(Circular_Buffer* cbuffer){
    free(cbuffer->cbuffer_node);
}

int CBUFFER_Is_Full(Circular_Buffer* cbuffer){
    if (cbuffer->is_state == FULL){
        return 1;
    }
    else{
        return 0;
    }
}

int CBUFFER_Is_Empty(Circular_Buffer* cbuffer){

    if (cbuffer->is_state == EMPTY){
        return 1;
    }
    else{
        return 0;
    }
}

// neither full nor empty
int CBUFFER_Is_Available(Circular_Buffer* cbuffer){
    if (cbuffer->is_state == AVAILABLE || cbuffer->is_state == EMPTY){
        return 1;
    }
    else{
        return 0;
    }
}

int CBUFFER_Add(Circular_Buffer* cbuffer, int socket, int type, char* ip_address){

    // if buffer full return error
    if (CBUFFER_Is_Full(cbuffer)){
        printf("CBUFFER is full\n");
        return -1;
    }
    else{
        int head = cbuffer->head;
        int tail = cbuffer->tail;

        cbuffer->cbuffer_node[head].socket = socket;
        cbuffer->cbuffer_node[head].type = type;
        memset(cbuffer->cbuffer_node[head].ip_address, 0, 32);
        strcpy(cbuffer->cbuffer_node[head].ip_address, ip_address);
        
        // update head
        head = (head+1) % cbuffer->size;

        // if head reached tail then buffer is full
        if ( head == tail )
            cbuffer->is_state = FULL;
        else   
            cbuffer->is_state = AVAILABLE;

        // update head
        cbuffer->head = head;

        return 1;
    }

}


// add with syncronization
int CBUFFER_Add_sync(Circular_Buffer* cbuffer, int socket, int type, char* ip_address){

    pthread_mutex_lock(cbuffer->cbuffer_mutex);

    while( CBUFFER_Is_Full(cbuffer)){
        // printf("found buffer full!! \n\n");
        // wait to empy signal
        pthread_cond_wait(cbuffer->cbuffer_empty_condition, cbuffer->cbuffer_mutex);
    }

    CBUFFER_Add(cbuffer, socket, type, ip_address);
    
    pthread_mutex_unlock(cbuffer->cbuffer_mutex);
    pthread_cond_signal(cbuffer->cbuffer_full_condition);
}

// pop with syncronization
int CBUFFER_Pop_sync(Circular_Buffer* cbuffer, int* socket, int* type, char* ip_address){
    pthread_mutex_lock(cbuffer->cbuffer_mutex);

    // check also for signal
    while ( CBUFFER_Is_Empty(cbuffer) && signal_occured == 0 ){
        // printf("found buffer empty!!! %ld \n\n", pthread_self()%10);
        // wait till full signal
        pthread_cond_wait(cbuffer->cbuffer_full_condition, cbuffer->cbuffer_mutex);
    }

    CBUFFER_Pop(cbuffer, socket, type, ip_address);

    pthread_mutex_unlock(cbuffer->cbuffer_mutex);
    pthread_cond_signal(cbuffer->cbuffer_empty_condition);
}

int CBUFFER_Pop(Circular_Buffer* cbuffer, int* socket, int* type, char* ip_address){

    // if buffer full return error
    if (CBUFFER_Is_Empty(cbuffer)){
        printf("CBUFFER is empty\n");
        return -1;
    }
    else{
        int head = cbuffer->head;
        int tail = cbuffer->tail;

        *socket = cbuffer->cbuffer_node[tail].socket;
        *type = cbuffer->cbuffer_node[tail].type;
        strcpy(ip_address, cbuffer->cbuffer_node[tail].ip_address);
        // *ip_address = cbuffer->cbuffer_node[tail].ip_address;
        
        // update tail
        tail = (tail+1) % cbuffer->size;

        // if head reached tail then buffer is empty
        if ( head == tail )
            cbuffer->is_state = EMPTY;
        else   
            cbuffer->is_state = AVAILABLE;

        // update head
        cbuffer->tail = tail;

        return 1;
    }

}

void CBUFFER_Print(Circular_Buffer* cbuffer){

    for (int i = 0; i < cbuffer->size; i++){
        printf("i(%d) sock %d typ %d  ip %s", i, cbuffer->cbuffer_node[i].socket, cbuffer->cbuffer_node[i].type, cbuffer->cbuffer_node[i].ip_address);
        if( i == cbuffer->head)
            printf(" head ");
        if( i == cbuffer->tail )
            printf(" tail ");
        printf("\n");
    }   
    

}

// init port stack
// last item is null
void WINFO_Init(Workers_Info* stack){

    stack->num_of_workers = 0;
    stack->port = malloc(sizeof(int));
    stack->port[0] = -1;
    stack->id = malloc(sizeof(int));
    stack->id[0] = -1;
    stack->size = 1;
    stack->ip_address = NULL;
}

void WINFO_Init_sync(Workers_Info* stack, pthread_mutex_t* worker_info_mutex){
    stack->num_of_workers = 0;
    stack->port = malloc(sizeof(int));
    stack->port[0] = -1;
    stack->id = malloc(sizeof(int));
    stack->id[0] = -1;
    stack->size = 1;
    stack->ip_address = NULL;

    // init mutex
    stack->worker_info_mutex = worker_info_mutex;
}


void WINFO_Destroy(Workers_Info* stack){
    free(stack->port);
    free(stack->id);
    free(stack->ip_address);
}

// when inserting id must be unique
// other wise it will overight the port of the existing id in case that a worker dies and resends its port number
int WINFO_Insert(Workers_Info* stack, int port, char* ip_address, int id){

    // search if id exists
    int id_exists = -1;
    int i = 0;
    for (i = 0; i < stack->num_of_workers; i++){
        if (stack->id[i] == id){
            id_exists = i;
            break;
        }
    }
    
    // if id does not exist
    if (id_exists == -1) {
        // reallocate and add -1 in the end
        stack->port = realloc(stack->port, (stack->size + 1)*sizeof(int));
        stack->port[stack->size-1] = port;
        stack->port[stack->size] = -1;
        // reallocate for the ids
        stack->id = realloc(stack->id, (stack->size + 1)*sizeof(int));
        stack->id[stack->size-1] = id;
        stack->id[stack->size] = -1;

        stack->num_of_workers++;
        stack->size++;
        // copy ip address
        if (stack->ip_address == NULL){
            stack->ip_address = malloc((strlen(ip_address)+1)*sizeof(char));
            strcpy(stack->ip_address, ip_address);
        }
        printf("insrted %d\n",id);
    }
    // if exists
    else{
        stack->port[id_exists] = port;
    }

}

// synced version
// when inserting id must be unique
// other wise it will overight the port of the existing id in case that a worker dies and resends its port number
int WINFO_Insert_sync(Workers_Info* stack, int port, char* ip_address, int id){
    // mutex lock
    pthread_mutex_lock(stack->worker_info_mutex);

    // search if id exists
    int id_exists = -1;
    int i = 0;
    for (i = 0; i < stack->num_of_workers; i++){
        if (stack->id[i] == id){
            id_exists = i;
            break;
        }
    }
    
    // if id does not exist
    if (id_exists == -1) {
        // reallocate and add -1 in the end
        stack->port = realloc(stack->port, (stack->size + 1)*sizeof(int));
        stack->port[stack->size-1] = port;
        stack->port[stack->size] = -1;
        // reallocate for the ids
        stack->id = realloc(stack->id, (stack->size + 1)*sizeof(int));
        stack->id[stack->size-1] = id;
        stack->id[stack->size] = -1;

        stack->num_of_workers++;
        stack->size++;
        // copy ip address
        if (stack->ip_address == NULL){
            stack->ip_address = malloc((strlen(ip_address)+1)*sizeof(char));
            strcpy(stack->ip_address, ip_address);
        }
        printf("insrted %d\n",id);
    }
    // if exists
    else{
        stack->port[id_exists] = port;
    }

    // unlock
    pthread_mutex_unlock(stack->worker_info_mutex);
}

int WINFO_Get_Ports_Copy_sync(Workers_Info* stack, int** copy_){
    // lock mutex
    pthread_mutex_lock(stack->worker_info_mutex);

    // copy the ports to given pointer
    int* copy = malloc(stack->size*sizeof(int));
    memcpy(copy, stack->port, stack->size*sizeof(int));
    *copy_ = copy;

    // unlock
    pthread_mutex_unlock(stack->worker_info_mutex);
}

int WINFO_Get_num_of_workers_sync(Workers_Info* stack, int* num_of_workers){
    pthread_mutex_lock(stack->worker_info_mutex);

    *num_of_workers = stack->num_of_workers;

    pthread_mutex_unlock(stack->worker_info_mutex);
}

int* WINFO_Get_Stack(Workers_Info* stack){
    return stack->port;
}