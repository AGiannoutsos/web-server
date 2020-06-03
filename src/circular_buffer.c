#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "../includes/circular_buffer.h"


// init circular buffer
void CBUFFER_Init(Circular_Buffer* cbuffer, int cbuffer_size){

    memset(cbuffer, 0, sizeof(Circular_Buffer));
    cbuffer->size = cbuffer_size;
    cbuffer->head = 0;
    cbuffer->tail = 0;
    cbuffer->is_state = EMPTY;

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
    if (cbuffer->is_state == AVAILABLE){
        return 1;
    }
    else{
        return 0;
    }
}

int CBUFFER_Add(Circular_Buffer* cbuffer, int socket, int type){

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

int CBUFFER_Pop(Circular_Buffer* cbuffer, int* socket, int* type){

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
        printf("i(%d) sock %d typ %d", i, cbuffer->cbuffer_node[i].socket, cbuffer->cbuffer_node[i].type);
        if( i == cbuffer->head)
            printf(" head ");
        if( i == cbuffer->tail )
            printf(" tail ");
        printf("\n");
    }   
    

}