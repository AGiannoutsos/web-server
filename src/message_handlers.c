#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>

#include "../includes/message_handlers.h"
#include "../includes/util.h"


int number_of_args(char* message){
    char *save_pointer;
    int num_of_words = 0;
    char* word;
    char* temp_message = malloc((strlen(message)+1)*sizeof(char));
    //find the num of arguments
    strcpy(temp_message,message);
    word = strtok_r(temp_message," \n", &save_pointer);
    while(word != NULL){
        word = strtok_r(NULL," \n", &save_pointer);
        num_of_words++;
    }

    free(temp_message);
    return num_of_words;
}

void Message_Print(Message_vector* message_vector){
    for (int i = 0; i < message_vector->num_of_args; i++){
        printf("> %s\n",message_vector->args[i]);
    }
    
}

void Message_Print_results(Message_vector* results){
    int num_of_results = results->num_of_args;

    // if results ar binary then change the print format
    if (num_of_results%2 == 0 ){
        for (int i = 0; i < num_of_results; i+=2){
            printf("%s %s \n",results->args[i], results->args[i+1]);
        }
    }
    else{
        for (int i = 0; i < num_of_results; i++){
            printf("%s ", results->args[i]);
        }
        printf("\n");
    }
}

// chech the message if end of message e*e*e and start of message s*s*s are included int he mesage strig
// if yes then it means that a complete message is sent
//  then strip unwanted string from the message and return 1
// if not then return -1
int Message_Decoder(Message_vector* message_vector){
    if (message_vector->num_of_args == 0)
        return -1;

    int num_of_args = message_vector->num_of_args;
    int message_exists = 0;
    int i = 0;
    int j = 0;
    for (i = 0; i < num_of_args; i++){
        // if found start search for end
        if (strcmp(message_vector->args[i], MSG_START) == 0){
            for (j = i; j < num_of_args; j++){
                if (strcmp(message_vector->args[j], MSG_END) == 0){
                    message_exists = 1;
                    break;
                }
            }
            if (message_exists == 1)
                break;
        }
    }
    if(message_exists == 0){
        return -1;
    }
    // else strip the unwanted part of the message
    else{
        Message_vector striped_message;
        Message_Init(&striped_message);
        Message_Copy(&striped_message, message_vector, i, num_of_args -(i+j+1));
        Message_Delete(message_vector);
        Message_Copy(message_vector, &striped_message , 0, 0);
        Message_Delete(&striped_message);
        return 1;
    }
    
    
}

// copy src to dest. Set offset n to excludde the n first and n last arguments of the message
void Message_Copy(Message_vector* dest, Message_vector* src, int upper_offset, int lower_offset){
    // copy message src to new one dest

    // delete dest
    Message_Delete(dest);
    int total_offset = lower_offset+upper_offset ;

    dest->num_of_args = src->num_of_args - total_offset;
    dest->args = malloc((dest->num_of_args)*sizeof(char*));

    for (int i = 0; i < dest->num_of_args; i++){
        dest->args[i] = malloc( (strlen(src->args[i+upper_offset])+1) *sizeof(char));
        strcpy(dest->args[i],src->args[i+upper_offset]);
    }
}

void Message_Init(Message_vector* message_vector){
    message_vector->num_of_args = 0;
    message_vector->args = NULL;        
}

// delete
void Message_Delete(Message_vector* message_vector){
    if (message_vector->args == NULL){
        message_vector->num_of_args = 0;
        return;
    }
    for (int i = 0; i < message_vector->num_of_args; i++){
        free(message_vector->args[i]);
    }
    free(message_vector->args);
    message_vector->num_of_args = 0;
    message_vector->args = NULL;
}

// append message_to_append to message and delete it
void Message_Append(Message_vector* message, Message_vector* message_to_append){
    int new_message_len = message->num_of_args + message_to_append->num_of_args;

    // reallocate space for new message
    message->args = realloc(message->args, new_message_len*sizeof(char*));


    // append the new words to the newly allocated space
    for (int i = 0; i < message_to_append->num_of_args; i++){
        message->args[i+message->num_of_args] = malloc((strlen(message_to_append->args[i])+1)*sizeof(char));
        strcpy(message->args[i+message->num_of_args], message_to_append->args[i]);
    }

    message->num_of_args = new_message_len;
    Message_Delete(message_to_append);
}

// conver string to Message_vector
int Message_to_vector(char* message, Message_vector* message_vector){
    // define the arguments of the message
    // and convert string to vector of strings
    int num_of_args;
    char* arg;
    char** args;
    char* temp_message;
    char* save_pointer;
    Message_Delete(message_vector);
    // chanhe temp message with strtok


    // find end of message or copy the whole buffer
    char* end_of_message = strstr(message, MSG_END);
    if(end_of_message != NULL){
        // find the end of message point
        int short_length = (int)(end_of_message-message) + strlen(MSG_END) + 1;
        temp_message = malloc(short_length*sizeof(char));
        memset(temp_message, 0, short_length*sizeof(char));
        memcpy(temp_message, message, short_length-1);
        temp_message[short_length-1] = '\0'; 
        num_of_args = number_of_args(temp_message);

    }
    else{
        temp_message = malloc((strlen(message)+1)*sizeof(char));
        memset(temp_message, 0, (strlen(message)+1)*sizeof(char));
        strcpy(temp_message, message);
        num_of_args = number_of_args(temp_message);
    }

    // allocate memory for args
    // only if arguments available
    if (num_of_args > 0){
        args = malloc(num_of_args*sizeof(char*)); 
        arg = strtok_r(temp_message," \n", &save_pointer);
        args[0] = malloc((strlen(arg)+1)*sizeof(char));
        strcpy(args[0],arg);
        for (int i = 1; i < num_of_args; i++){
            arg = strtok_r(NULL," \n", &save_pointer);
            // printf("arg -> %s\n",arg);
            args[i] = malloc((strlen(arg)+1)*sizeof(char));
            strcpy(args[i],arg);
            // if(strcmp(arg, MSG_END) == 0)
            //     break;
        }
        message_vector->args = args;
        message_vector->num_of_args = num_of_args;
    }
    else{
        message_vector->args = NULL;
        message_vector->num_of_args = 0;
    }
    free(temp_message);

}

void Message_Create_buffer_offset_(char** buffer, int** offset, int num_of_fds){
    *offset = malloc(num_of_fds*sizeof(int));
    memset(*offset, 0, num_of_fds*sizeof(int));
    buffer[0] = malloc(READ_BUFFER_SIZE*sizeof(char));
    memset(buffer[0], 0, READ_BUFFER_SIZE*sizeof(char));
}

void Message_Destroy_buffer_offset_(char** buffer, int** offset, int num_of_fds){
    free(*buffer);
    free(*offset);
}

void Message_Create_buffer_offset(Buffer** _buffer, int** _offset, int num_of_workers){
    int* previous_offset = malloc(num_of_workers*sizeof(int));
    memset(previous_offset, 0, num_of_workers*sizeof(int));

    // buffer size + 1 for signal queue
    Buffer* buffer = malloc((num_of_workers)*sizeof(Buffer));
    memset(buffer, 0, (num_of_workers)*sizeof(Buffer));
    for (int i = 0; i < (num_of_workers); i++){
        buffer[i].buffer_size = READ_BUFFER_SIZE;
        buffer[i].buffer = malloc(READ_BUFFER_SIZE*sizeof(char));
        memset(buffer[i].buffer, 0, READ_BUFFER_SIZE*sizeof(char));
    }
    // copy pointer of new buffer to _buffer
    *_buffer = buffer;
    *_offset = previous_offset;

}

void Message_Destroy_buffer_offset(Buffer** buffer, int** offset, int num_of_fds){
    for (int i = 0; i < (num_of_fds); i++){
        free((*buffer)[i].buffer);
    }
    free(*offset);
    free(*buffer);
}


int Message_to_string(Message_vector* message, char** string){
    // find and allocate the space needed for the string
    // clear string to write on
    // char* string;
    int string_length = 0;
    int offset = 0;
    for (int i = 0; i < message->num_of_args; i++){
        string_length += strlen(message->args[i])+1;
    }
    // add and the start and end of message
    string_length += strlen(MSG_START)+1;
    string_length += strlen(MSG_END)+1;
    // for better or worse...
    string_length+=3;

    *string = malloc(string_length*sizeof(char));

    // copy every arg on the string and calculate the offset
    sprintf((*string), "%s", MSG_START);
    offset += strlen(MSG_START);

    for (int i = 0; i < message->num_of_args; i++){
        sprintf((*string)+offset, " %s", message->args[i]);
        offset += strlen(message->args[i])+1;
    }
    
    sprintf((*string)+offset, " %s ", MSG_END);

}

// read string while you find a message
int Message_Read_string(int fd, int bytes_write, Message_vector* message, Buffer* buffer, int buffer_size){
    int bytes_read = 0;
    char inter_buffer[buffer_size];
    memset(inter_buffer, '\0', buffer_size);
    int new_read_buffer_size;
    char* new_buffer;
    int read_buffer_size = READ_BUFFER_SIZE;

    // printf("pipes 1 %d %d \n", fd, errno);
    while( (bytes_read = read(fd, inter_buffer, buffer_size)) > 0){
    // printf("pipes 2 %d %d %d\n", fd, errno, bytes_read);
    // usleep(100000);
    // printf("\n>>> %.*s <<<\n", bytes_read, inter_buffer);
        // if buffer size is exceded then copy the previous one and allovate more
        if( bytes_write + bytes_read > buffer->buffer_size){
            new_read_buffer_size = (buffer->buffer_size + bytes_write + bytes_read) * READ_BUFFER_SIZE_INCREASE_FACTOR;
            new_buffer = malloc(new_read_buffer_size*sizeof(char));
            memset(new_buffer, 0, new_read_buffer_size*sizeof(char));
            // coppy old to new
            memcpy(new_buffer, buffer->buffer, buffer->buffer_size);
            // free old
            free(buffer->buffer);
            // arrange the pointers
            buffer->buffer = new_buffer;
            buffer->buffer_size = new_read_buffer_size;
        }

        // check bytes and write +offset
        memcpy(buffer->buffer+bytes_write, inter_buffer, bytes_read);

        bytes_write += bytes_read;
        // flush intermediate buffer
        memset(inter_buffer, '\0', buffer_size);

        Message_Delete(message);
        // Check if message is available
        Message_to_vector(buffer->buffer, message);
        // if message available then
        // clean buffer till the first message
        // reset and return offset of buffer
        if (Message_Decoder(message) > 0){ 
            break;
        }
    }
    return bytes_write;
}

extern int errno;
// read from pipe and store read in a bufer array with end of string '\0'
// the decode the message and return it
int* Message_Read_from_one(int* fds, int num_of_fd, Message_vector* _message, Buffer* buffer, int* previous_offset, int buffer_size){
    int fd = num_of_fd;
    int bytes_read = 0;
    char* end_of_message;
    int* bytes_write = previous_offset;
    // char inter_buffer[buffer_size];
    // memset(inter_buffer, '\0', buffer_size);
    
    Message_vector message;
    Message_Init(&message);

    fd_set fd_set;
    get_fd_set(&fd_set, &fds[fd], 1);

    // set timer
    struct timeval timeval;
    timeval.tv_sec = 0;
    timeval.tv_usec = 0;


    // Select if pipe haw data to offer then sleep
    int select_output = 0;
    int exit_select = 0;
    while(exit_select == 0){
        
        // wait 200 ms and check again the pipe
        select_output = select(fds[fd]+1, &fd_set, NULL, NULL, &timeval);
        // printf("ma to 8eo %d %d  %d   %d    %d\n",select_output, fds[0], pthread_self(), bytes_write[fd], errno); fflush(stdout);

        // if pipe is ready then select it to read
        if( select_output > 0 ){  
            // while bytes available in pipe read and store to an intermediate buffer
            // and then copy them on the inserted buffer
            bytes_write[fd] = Message_Read_string(fds[fd], bytes_write[fd], &message, &buffer[fd], buffer_size);
            // if read is unavailable but buffer is the get data from it
            // in case of large buffer size but small messages
            if(bytes_write[fd] > 0 ){
                Message_Delete(&message);
                Message_to_vector(buffer[fd].buffer, &message);
                if (Message_Decoder(&message) > 0){ 
                    end_of_message = strstr(buffer[fd].buffer, MSG_END);
                    end_of_message += strlen(MSG_END);
                    // reset offeset of bytews written
                    bytes_write[fd] = (int)(buffer[fd].buffer - end_of_message) + bytes_write[fd];

                    memmove(buffer[fd].buffer, end_of_message, bytes_write[fd]);
                    // clear the buffer from dirty bytes
                    memset(buffer[fd].buffer+bytes_write[fd], '\0', buffer[fd].buffer_size-bytes_write[fd]);
                    // reset offset;
                    exit_select = 1;
                }
            }
        }
        else{
            // reset timer for long awaited input
            get_fd_set(&fd_set, &fds[fd], 1);
            timeval.tv_sec = TIMEOUT_SEC;
            timeval.tv_usec = TIMEOUT_USEC*2;
            // pipe is not available but i have avoilable data in buffer
            if(bytes_write[fd] > 1 ){
                Message_Delete(&message);
                Message_to_vector(buffer[fd].buffer, &message);
                // set timeout to 0 because there are still data in the buffer
                timeval.tv_sec = 0;
                timeval.tv_usec = 0;
                if (Message_Decoder(&message) > 0){ 
                    end_of_message = strstr(buffer[fd].buffer, MSG_END);
                    end_of_message += strlen(MSG_END);
                    // reset offeset of bytews written
                    bytes_write[fd] = (int)(buffer[fd].buffer - end_of_message) + bytes_write[fd];

                    memmove(buffer[fd].buffer, end_of_message, bytes_write[fd]);
                    // clear the buffer from dirty bytes
                    memset(buffer[fd].buffer+bytes_write[fd], '\0', buffer[fd].buffer_size-bytes_write[fd]);
                    // reset offset;
                    exit_select = 1;
                }
            }
        }

   }
    
    // copy messsage and remove MSG_END MSG_START 
    Message_Delete(_message);
    Message_Copy(_message, &message, 1, 1);
    Message_Delete(&message);

    return bytes_write;
}

int Message_Read_string_socket(int fd, int bytes_write, Message_vector* message, Buffer* buffer, int buffer_size){
    int bytes_read = 0;
    char inter_buffer[buffer_size];
    memset(inter_buffer, '\0', buffer_size);
    int new_read_buffer_size;
    char* new_buffer;
    int read_buffer_size = READ_BUFFER_SIZE;

            // printf("opaa -> %d  %d  %d\n",bytes_write,fd,  errno);


    Message_Delete(message);
    // Check if message is available
    Message_to_vector(buffer->buffer, message);
    // if message available then
    // clean buffer till the first message
    // reset and return offset of buffer
    if (Message_Decoder(message) > 0){ 
        return bytes_write;
    }


    while( (bytes_read = read(fd, inter_buffer, buffer_size)) > 0){
        // if buffer size is exceded then copy the previous one and allovate more
        if( bytes_write + bytes_read > buffer->buffer_size){
            new_read_buffer_size = (buffer->buffer_size + bytes_write + bytes_read) * READ_BUFFER_SIZE_INCREASE_FACTOR;
            new_buffer = malloc(new_read_buffer_size*sizeof(char));
            memset(new_buffer, 0, new_read_buffer_size*sizeof(char));
            // coppy old to new
            memcpy(new_buffer, buffer->buffer, buffer->buffer_size);
            // free old
            free(buffer->buffer);
            // arrange the pointers
            buffer->buffer = new_buffer;
            buffer->buffer_size = new_read_buffer_size;
        }

        // check bytes and write +offset
        memcpy(buffer->buffer+bytes_write, inter_buffer, bytes_read);

        bytes_write += bytes_read;
        // flush intermediate buffer
        memset(inter_buffer, '\0', buffer_size);

        Message_Delete(message);
        // Check if message is available
        Message_to_vector(buffer->buffer, message);
        // if message available then
        // clean buffer till the first message
        // reset and return offset of buffer
        if (Message_Decoder(message) > 0){ 
            break;
        }
    }
    printf("opaa ->>> %d  %d  %d\n",bytes_read,fd, errno);
    return bytes_write;
}

int* Message_Read_from_one_socket(int* fds, int num_of_fd, Message_vector* _message, Buffer* buffer, int* previous_offset, int buffer_size){
    int fd = num_of_fd;
    int bytes_read = 0;
    char* end_of_message;
    int* bytes_write = previous_offset;
    // char inter_buffer[buffer_size];
    // memset(inter_buffer, '\0', buffer_size);
    
    Message_vector message;
    Message_Init(&message);

    fd_set fd_set, read_set;
    get_fd_set(&fd_set, &fds[fd], 1);

    // set timer
    struct timeval timeval;
    timeval.tv_sec = TIMEOUT_SEC;
    timeval.tv_usec = TIMEOUT_USEC;

    // Select if pipe haw data to offer then sleep
    int select_output = 0;
    int exit_select = 0;
    while(exit_select == 0){
        read_set = fd_set;
        timeval.tv_sec = TIMEOUT_SEC;
        timeval.tv_usec = TIMEOUT_USEC;

        // wait 200 ms and check again the pipe
        // select_output = select(fds[fd]+1, &read_set, NULL, NULL, &timeval);
        // printf("ma to 8eo socket %d  %d  %d   %d\n",select_output, fds[0], pthread_self(), bytes_write[fd]); fflush(stdout);
        // if pipe is ready then select it to read
        // if( select_output > 0 ){  


            // while bytes available in pipe read and store to an intermediate buffer
            // and then copy them on the inserted buffer
            bytes_write[fd] = Message_Read_string_socket(fds[fd], bytes_write[fd], &message, &buffer[fd], buffer_size);
            // if read is unavailable but buffer is the get data from it
            // in case of large buffer size but small messages
            if(bytes_write[fd] > 0 ){
                Message_Delete(&message);
                Message_to_vector(buffer[fd].buffer, &message);
                // Message_Print(&message);
                if (Message_Decoder(&message) > 0){ 
                    end_of_message = strstr(buffer[fd].buffer, MSG_END);
                    end_of_message += strlen(MSG_END);
                    // reset offeset of bytews written
                    bytes_write[fd] = (int)(buffer[fd].buffer - end_of_message) + bytes_write[fd];

                    memmove(buffer[fd].buffer, end_of_message, bytes_write[fd]);
                    // clear the buffer from dirty bytes
                    memset(buffer[fd].buffer+bytes_write[fd], '\0', buffer[fd].buffer_size-bytes_write[fd]);
                    // reset offset;
                    exit_select = 1;
                }
            }
        // }
        // else{
        //     // reset timer for long awaited input
        //     get_fd_set(&fd_set, &fds[fd], 1);
        //     timeval.tv_sec = TIMEOUT_SEC;
        //     timeval.tv_usec = TIMEOUT_USEC;
        //     // pipe is not available but i have avoilable data in buffer
        //     if(bytes_write[fd] > 1 ){
        //         Message_Delete(&message);
        //         Message_to_vector(buffer[fd].buffer, &message);
        //         // set timeout to 0 because there are still data in the buffer
        //         timeval.tv_sec = 0;
        //         timeval.tv_usec = 0;
        //         if (Message_Decoder(&message) > 0){ 
        //             end_of_message = strstr(buffer[fd].buffer, MSG_END);
        //             end_of_message += strlen(MSG_END);
        //             // reset offeset of bytews written
        //             bytes_write[fd] = (int)(buffer[fd].buffer - end_of_message) + bytes_write[fd];

        //             memmove(buffer[fd].buffer, end_of_message, bytes_write[fd]);
        //             // clear the buffer from dirty bytes
        //             memset(buffer[fd].buffer+bytes_write[fd], '\0', buffer[fd].buffer_size-bytes_write[fd]);
        //             // reset offset;
        //             exit_select = 1;
        //         }
        //     }
        // }

   }
    
    // copy messsage and remove MSG_END MSG_START 
    Message_Delete(_message);
    Message_Copy(_message, &message, 1, 1);
    Message_Delete(&message);

    return bytes_write;
}


int Message_Is_End_Com(Message_vector* message, int num_of_fds){
    if (message->args == NULL)
        return 0;
    int end_coms = 0;
    for (int i = 0; i < num_of_fds; i++){
        
        if( message[i].num_of_args == 1 && strcmp(message[i].args[0], END_COM) == 0  ){
            end_coms++;
        }
        else{
            return 0;
        }

    }
    if( end_coms == num_of_fds)
        return 1;
    else
        return 0;
    
}

int Message_Write_End_Com(int fd, int buffer_size){
    // a buffer string to store vector message and transfer it
    char* string;
    int bytes_write;
    Message_vector end_com;
    Message_Init(&end_com);
    end_com.num_of_args = 1;
    end_com.args = malloc(sizeof(char*));
    end_com.args[0] = malloc((strlen(END_COM)+1)*sizeof(char));
    strcpy(end_com.args[0], END_COM);
    // conver message to string
    Message_to_string(&end_com, &string);
    // send string to pipe
    bytes_write = Message_Write_string(fd, string, buffer_size);

    Message_Delete(&end_com);
    free(string);
    return bytes_write;
}

int Message_Write_to_many(int* fds,int num_of_fds, Message_vector* message, int buffer_size){
    for (int i = 0; i < num_of_fds; i++){
        Message_Write(fds[i], message, buffer_size);
    }
}

int Message_Write(int fd, Message_vector* message, int buffer_size){
    // a buffer string to store vector message and transfer it
    char* string;
    int bytes_write;
    // conver message to string
    Message_to_string(message, &string);
    // send string to pipe
    bytes_write = Message_Write_string(fd, string, buffer_size);

    free(string);
    return bytes_write;
}

// write to pipe string, buffer must contain end of string '\0'
int Message_Write_string(int fd, char* buffer, int buffer_size){
    int bytes_write = 0;
    int total_bytes_write = 0;
    int message_len = 0;

    message_len = strlen(buffer);
    // while message exist send buffer_size chunks of data
    // fprintf(stderr,"message len %d  fd  %d\n %s \n", message_len, fd, buffer);
    while (message_len > 0){
        // if message length < buffer size then send less bytes
        if (message_len < buffer_size){
            bytes_write = write(fd, buffer+total_bytes_write, message_len);
            message_len -= bytes_write;
            total_bytes_write += bytes_write;
        }
        // else send message and always reset the offset
        else{
            bytes_write = write(fd, buffer+total_bytes_write, buffer_size);
            message_len -= bytes_write;
            total_bytes_write += bytes_write;
        }
    }

    // if error return error else reryrn total bytes
    if (bytes_write < 0){
        perror("write");
        return -1;
    }
    return total_bytes_write;
}