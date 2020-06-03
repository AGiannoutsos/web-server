#ifndef MESSAGE_HANDLERS_H
#define MESSAGE_HANDLERS_H

#include <signal.h>

#define MSG_START "s*s"
#define MSG_END "e*e"
#define END_COM "e*c"

#define TIMEOUT_SEC 0
#define TIMEOUT_USEC 100000

#define READ_BUFFER_SIZE 4096
#define READ_BUFFER_SIZE_INCREASE_FACTOR 2

typedef struct Message_vector{

    int num_of_args;
    char** args;

} Message_vector;

typedef struct Buffer{
    char* buffer;
    int buffer_size;
} Buffer;



int number_of_args(char* message);

// Message vector utility functions
void Message_Delete(Message_vector* message_vector);
void Message_Init(Message_vector* message_vector);
void Message_Copy(Message_vector* dest, Message_vector* src, int upper_offset, int lower_offset);
void Message_Append(Message_vector* message, Message_vector* message_to_append);

void Message_Create_buffer_offset(Buffer** _buffer, int** _offset, int num_of_workers);
void Message_Destroy_buffer_offset(Buffer** buffer, int** offset, int num_of_fds);

// utility functions for communication
int Message_Decoder(Message_vector* message_vector);
int Message_Is_End_Com(Message_vector* message, int num_of_fds);
int Message_Write_End_Com(int fd, int buffer_size);
int Message_Write_string(int fd, char* buffer, int buffer_size);
int Message_to_vector(char* message, Message_vector* message_vector);
int Message_to_string(Message_vector* message, char** string);
void Message_Print(Message_vector* message_vector);

// send message function
int Message_Write(int fd, Message_vector* message, int buffer_size);
int Message_Write_to_many(int* fds,int num_of_fds, Message_vector* message, int buffer_size);

// read message function
int* Message_Read_from_one(int* fd, int num_of_fd, Message_vector* _message, Buffer* buffer, int* previous_offset, int buffer_size);
int Message_Read_string(int fd, int bytes_write, Message_vector* message, Buffer* buffer, int buffer_size);

#endif