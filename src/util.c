#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>

#include "../includes/util.h"
#include "../includes/message_handlers.h"

int max(int *array, int items){
    int max = 0;
    for (int i = 0; i < items; i++){
        if(array[i] > max)
            max = array[i];
    }
    return max;
}

int sum(int *s, int n){
    int sum = 0;
    for (int i = 0; i < n; i++){
        sum+=s[i];
    }
    return sum;
}

int get_fd_set(fd_set* fd_set, int* fds, int num_of_fds){
    FD_ZERO(fd_set);
    for (int i = 0; i < num_of_fds; i++){
        FD_SET(fds[i], fd_set);
    }
    return num_of_fds;
}

// handle signal queue
volatile sig_atomic_t num_of_signals = 0;
int add_signal(volatile sig_atomic_t* buffer, volatile sig_atomic_t* sender_pid_buffer, int signal, int pid){
    // queue with limited size
    if (num_of_signals <= READ_BUFFER_SIZE){
        sender_pid_buffer[num_of_signals] = pid;
        buffer[num_of_signals] = signal;
        num_of_signals++;
        
    }
}

volatile sig_atomic_t pop_signal(volatile sig_atomic_t* buffer, volatile sig_atomic_t* sender_pid_buffer, volatile sig_atomic_t* signal_sent_from_pid){
    if( num_of_signals ){
        volatile sig_atomic_t signal = buffer[0];
        volatile sig_atomic_t sender_pid = sender_pid_buffer[0];
        // move queue
        memmove(sender_pid_buffer, &(sender_pid_buffer[1]), READ_BUFFER_SIZE-num_of_signals);
        memmove(buffer, &(buffer[1]), READ_BUFFER_SIZE-num_of_signals);
        num_of_signals--;
        *signal_sent_from_pid = sender_pid;
        return signal;
    }
    return 0;
}


int get_directory_content(char* input_dir, int type, int*_num_of_directories, char*** _directories){
    DIR* directory;
    struct dirent *directory_content;
    int num_of_directories = 0;
    char** directories;

    // set directories their number must no be over workers
    directory = opendir(input_dir);
    if (directory == NULL){
        perror("opendir");
        exit(1);
    }
    while( (directory_content = readdir(directory)) != NULL ){
        // check and remove files and back directory
        if ( directory_content->d_type == type && strcmp(directory_content->d_name, ".") != 0 && strcmp(directory_content->d_name, "..") != 0 ){
            num_of_directories++;
        }
    }    
    closedir(directory);
    // malloc directories
    directories = malloc(num_of_directories*sizeof(char*));
    int curr_dir_num = 0;

    directory = opendir(input_dir);
    while( (directory_content = readdir(directory)) != NULL ){
        // check and remove files and back directory
        if ( directory_content->d_type == type && strcmp(directory_content->d_name, ".") != 0 && strcmp(directory_content->d_name, "..") != 0 ){
            directories[curr_dir_num] = malloc((strlen(directory_content->d_name)+1) * sizeof(char));
            strcpy(directories[curr_dir_num], directory_content->d_name);
            curr_dir_num++;
        }
    }    
    closedir(directory);

    *_num_of_directories = num_of_directories;
    *_directories = directories; 

}

int get_directories_per_worker_RR(Worker* worker, int num_of_workers, char** directories, int num_of_directories){

    // set directories per worker Round Robin
    for (int i = 0; i < num_of_directories; i++){
        worker[i%num_of_workers].num_of_directories++;
    }
    // allocate memory to add the directories
    for (int i = 0; i < num_of_workers; i++){
        worker[i].directories = malloc(worker[i].num_of_directories*sizeof(char*));
    }
    // add directories to struct
    int dirs = 0;
    int dirs_position = 0;
    for (int i = 0; i < num_of_workers; i++){
        dirs = worker[i].num_of_directories;
        for (int j = 0; j < dirs; j++){
            worker[i].directories[j] = malloc((strlen(directories[dirs_position])+1)*sizeof(char));
            strcpy(worker[i].directories[j], directories[dirs_position]); 
            dirs_position++;
        }
        // dirs_position += dirs;
    }
    
    return num_of_workers;
}

void update_statistics_dates(char** age_range_ptr, Message_vector * file_content){
    // get age
    int age = atoi(file_content->args[5]);

    // select based on age range
    if( age <= 20 ){
        age_range_ptr += 0;
    }
    else if( age >= 21 && age <= 40){
        age_range_ptr += 1;
    }
    else if( age >= 41 && age <= 60){
        age_range_ptr += 2;
    }else{
        age_range_ptr += 3;
    }

    // update the cases
    int previous_cases = atoi(age_range_ptr[0]);
    free(age_range_ptr[0]);

    // allocate new cases and update it
    int new_cases = previous_cases + 1;
    char new_cases_string[64] = {0};
    sprintf(new_cases_string, "%d", new_cases);
    age_range_ptr[0] = malloc((strlen(new_cases_string)+1)+sizeof(int));
    strcpy(age_range_ptr[0], new_cases_string);
}

void create_new_statistics_disease(Message_vector* statistics, Message_vector * file_content){

        //  create new message then append it to current statistics
        Message_vector new_statistics;
        Message_Init(&new_statistics);
        new_statistics.num_of_args = 5;
        new_statistics.args = malloc(new_statistics.num_of_args*sizeof(char*));
        // allocate for the disease
        new_statistics.args[0] = malloc((strlen(file_content->args[4])+1)*sizeof(char));
        strcpy(new_statistics.args[0], file_content->args[4]);
        // allocate for age ranges
        for (int i = 1; i < 5; i++){
            new_statistics.args[i] = malloc(2*sizeof(char));
            strcpy(new_statistics.args[i], "0");
        }
        update_statistics_dates(&(new_statistics.args[1]), file_content);

        // append completed new_statistics message to the complete one
        Message_Append(statistics, &new_statistics);
}
// update statistics with statistics to update which is file content
void Statistics_Update(Message_vector* statistics, Message_vector * file_content, char* country, char* date){
    //  if  statistics == NULL then new stats are isnerted
    if(statistics->num_of_args == 0 || statistics->args == NULL){
        statistics->num_of_args = 2;
        statistics->args = malloc(2*sizeof(char*));
        // allocate country
        statistics->args[0] = malloc((strlen(country)+1)*sizeof(char));
        strcpy(statistics->args[0], country);
        // allocate date
        statistics->args[1] = malloc((strlen(date)+1)*sizeof(char));
        strcpy(statistics->args[1], date);
    }

    // scan to find if disease is already documented
    int found = 0;
    int i = 0;
    for (i = 2; i < statistics->num_of_args; i+=5){
        if( strcmp(statistics->args[i], file_content->args[4]) == 0){
            // disease found
            found = 1;
            break;
        }
    }
    // if found update the cases
    if(found){
        update_statistics_dates(&(statistics->args[i+1]), file_content);
    }
    // else create a new disease statistic
    else{
        create_new_statistics_disease(statistics, file_content);
    }

}

void Statistics_Print(Message_vector* statistics, FILE* output){
    if(statistics->num_of_args == 0 | statistics->args == NULL){
        return;
    }
    fprintf(output, "%s %d\n", statistics->args[0], statistics->num_of_args);
    fprintf(output, "%s\n", statistics->args[1]);

    for (int i = 2; i < statistics->num_of_args; i+=5){
        fprintf(output, "%s\n", statistics->args[i]);
        fprintf(output, "Age range 0-20 years: %s cases\n", statistics->args[i+1]);
        fprintf(output, "Age range 21-40 years: %s cases\n", statistics->args[i+2]);
        fprintf(output, "Age range 41-60 years: %s cases\n", statistics->args[i+3]);
        fprintf(output, "Age range 60+ years: %s cases\n", statistics->args[i+4]);     
        fprintf(output, "\n");
    }

}

int Worker_Init(Worker** worker, int num_of_workers){
    *worker = malloc(num_of_workers*sizeof(Worker));
    memset(*worker, 0, num_of_workers*sizeof(Worker));
}


int Worker_Destroy(Worker** worker, int num_of_workers){
    int num_of_dirs = 0;
    for (int i = 0; i < num_of_workers; i++){
        num_of_dirs = (*worker)[i].num_of_directories;
        for (int j = 0; j < num_of_dirs; j++){
            free((*worker)[i].directories[j]);
        }
        free((*worker)[i].directories);
        free((*worker)[i].server_ip);
        free((*worker)[i].server_port);
    }
    free(*worker);
}

int Worker_add_server_info(Worker* worker, int num_of_workers, char* server_ip, char* server_port){
    for (int i = 0; i < num_of_workers; i++){
        // copy server ip
        (worker)[i].server_ip = malloc((strlen(server_ip)+1)*sizeof(char));
        strcpy((worker)[i].server_ip, server_ip );

        // copy server port
        (worker)[i].server_port = malloc((strlen(server_port)+1)*sizeof(char));
        strcpy((worker)[i].server_port, server_port );
        
    }
}

int create_pipes(int* pipe_id, int num_of_workers){
    char pipe_name[64] = {0};
    int err;
    for (int i = 0; i < num_of_workers; i++){
        // worker.(pid)
        sprintf(pipe_name, "worker.%d", pipe_id[i]);
        err = mkfifo(pipe_name, 0666);
        if (err < 0)
            perror("create pipe");

        memset(pipe_name, 0, 64);
        // aggragator.(pid)
        sprintf(pipe_name, "master.%d", pipe_id[i]);
        err = mkfifo(pipe_name, 0666);
        if (err < 0)
            perror("create pipe");

        memset(pipe_name, 0, 64);
    }
    return num_of_workers;
}

int unlink_pipes(int* pipe_id, int num_of_workers){
    char pipe_name[64] = {0};
    int err;
    for (int i = 0; i < num_of_workers; i++){
        // worker.(pid)
        sprintf(pipe_name, "worker.%d", pipe_id[i]);
        err = unlink(pipe_name);
        if (err < 0)
            perror("unlink pipe");

        memset(pipe_name, 0, 64);
        // aggragator.(pid)
        sprintf(pipe_name, "master.%d", pipe_id[i]);
        err = unlink(pipe_name);
        if (err < 0)
            perror("unlink pipe");

        memset(pipe_name, 0, 64);
    }
    return num_of_workers;
}

// select mode and pipe type and return files descriptors
int open_pipes(int** _fds, char* pipe, int mode, int* pipe_id, int num_of_fds){
    char pipe_name[64] = {0};

    int* fds = malloc(num_of_fds*sizeof(int));
    memset(fds, 0, num_of_fds*sizeof(int));

    for (int i = 0; i < num_of_fds; i++){
        sprintf(pipe_name, "%s.%d",pipe, pipe_id[i]);
        // printf("open piee-> %s\n",pipe_name);
        fds[i] = open(pipe_name, mode);
        if (fds[i] < 0)
            perror("open pipe");
    }
    *_fds = fds;
    return num_of_fds;
}

int close_pipes(int** fds, char* pipe, int* pipe_id, int num_of_fds){
    char pipe_name[64] = {0};

    for (int i = 0; i < num_of_fds; i++){
        sprintf(pipe_name, "%s.%d",pipe, pipe_id[i]);
        // printf("close piee-> %s  %d\n",pipe_name, (*fds)[i]);
        close((*fds)[i]);
        if ((*fds)[i] < 0)
            perror("close pipe");
    }
    free(*fds);
    return num_of_fds;
}