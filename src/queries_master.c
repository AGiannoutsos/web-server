#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/message_handlers.h"
#include "../includes/util.h"
#include "../includes/queries_master.h"

int list_countries(Worker* worker, int num_of_workers){
    int num_of_directories = 0;
    for (int i = 0; i < num_of_workers; i++){
        num_of_directories = worker[i].num_of_directories;
        for (int j = 0; j < num_of_directories; j++){
            printf("%s %d\n",worker[i].directories[j], worker[i].pid);
        }
        
    }
    
}

int disease_frequency(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, int buffer_size){
    Message_Write_to_many(write_fds, num_of_workers, message, buffer_size);

    // get results
    int *previous_offset;
    Buffer* buffer;
    Message_Create_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_vector results;
    Message_Init(&results);
    // read from every worker
    // end if worker has sent End of Communication
    int fail = 0;
    int sum = 0;
    int result = 0;
    for (int i = 0; i < num_of_workers; i++){
        previous_offset = Message_Read_from_one(read_fds, i, &results, buffer, previous_offset, buffer_size);
        while(!Message_Is_End_Com(&results, 1)){
            result = atoi(results.args[0]);
            if (result < 0) // then query failed
                fail++;
            sum += atoi(results.args[0]);
            previous_offset = Message_Read_from_one(read_fds, i, &results, buffer, previous_offset, buffer_size);
        }
    }
    Message_Destroy_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_Delete(&results);

    if (fail > 0){ // then something failed
        return -1;
    }
    else{ // everything was correct
        printf("%d\n", sum); fflush(stdout);
        return 1;
    }
}


int topk_age_ranges(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, int buffer_size){
    Message_Write_to_many(write_fds, num_of_workers, message, buffer_size);

    // get results
    int *previous_offset;
    Buffer* buffer;
    Message_Create_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_vector results;
    Message_Init(&results);
    // read from every worker
    // end if worker has sent End of Communication
    int fail = 0;
    int sum = 0;
    int result = 0;
    int num_of_results = 0;
    for (int i = 0; i < num_of_workers; i++){
        previous_offset = Message_Read_from_one(read_fds, i, &results, buffer, previous_offset, buffer_size);
        while(!Message_Is_End_Com(&results, 1)){
            result = atoi(results.args[0]);
            num_of_results = results.num_of_args;
            if(num_of_results == 1){
                if (result < 0) // then query failed
                    fail++;
            }
            else{ // something was found
                for (int i = 0; i < num_of_results; i+=2){
                    printf("%s: %s%% \n",results.args[i], results.args[i+1]);
                }
            }
            previous_offset = Message_Read_from_one(read_fds, i, &results, buffer, previous_offset, buffer_size);
        }
    }
    Message_Destroy_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_Delete(&results);

    if (fail > 0){ // then something failed
        return -1;
    }
    else{ // everything was correct
        return 1;
    }
}


int search_patient_record(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, int buffer_size){
    Message_Write_to_many(write_fds, num_of_workers, message, buffer_size);

    // get results
    int *previous_offset;
    Buffer* buffer;
    Message_Create_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_vector results;
    Message_Init(&results);
    // read from every worker
    // end if worker has sent End of Communication
    int fail = 0;
    int sum = 0;
    int result = 0;
    int num_of_results = 0;
    for (int i = 0; i < num_of_workers; i++){
        previous_offset = Message_Read_from_one(read_fds, i, &results, buffer, previous_offset, buffer_size);
        while(!Message_Is_End_Com(&results, 1)){
            result = atoi(results.args[0]);
            num_of_results = results.num_of_args;
            if(num_of_results == 1){
                if (result < 0) // then query failed
                    fail++;
            }
            else{ // something was found
                printf("%s %s %s %s %s %s %s\n",results.args[0], results.args[1], results.args[2], results.args[3], results.args[4], results.args[5], results.args[6]);
            }
            previous_offset = Message_Read_from_one(read_fds, i, &results, buffer, previous_offset, buffer_size);
        }
    }
    Message_Destroy_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_Delete(&results);

    if (fail > 0){ // then something failed
        return -1;
    }
    else{ // everything was correct
        return 1;
    }
}

int num_patients_admissions_discharges(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, int buffer_size){
    Message_Write_to_many(write_fds, num_of_workers, message, buffer_size);

    // get results
    int *previous_offset;
    Buffer* buffer;
    Message_Create_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_vector results;
    Message_Init(&results);
    // read from every worker
    // end if worker has sent End of Communication
    int fail = 0;
    int sum = 0;
    int result = 0;
    int num_of_results = 0;
    for (int i = 0; i < num_of_workers; i++){
        previous_offset = Message_Read_from_one(read_fds, i, &results, buffer, previous_offset, buffer_size);
        while(!Message_Is_End_Com(&results, 1)){
            result = atoi(results.args[0]);
            num_of_results = results.num_of_args;
            if(num_of_results == 1){
                if (result < 0) // then query failed
                    fail++;
            }
            else{ // something was found
                for (int i = 0; i < num_of_results; i+=2){
                    printf("%s %s\n",results.args[i], results.args[i+1]);
                }
            }
            previous_offset = Message_Read_from_one(read_fds, i, &results, buffer, previous_offset, buffer_size);
        }
    }
    Message_Destroy_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_Delete(&results);

    if (fail > 0){ // then something failed
        return -1;
    }
    else{ // everything was correct
        return 1;
    }
}


// server version 
// same but also they copy results to results messae
int disease_frequency_server(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, Message_vector* results_printed, int buffer_size){
    Message_Write_to_many(write_fds, num_of_workers, message, buffer_size);
    // sleep(6);
    // init buffer to copy the printeed results to message results printed
    char results_string[64] = {0};

    // get results
    int *previous_offset;
    Buffer* buffer;
    Message_Create_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_vector results;
    Message_Init(&results);
    // read from every worker
    // end if worker has sent End of Communication
    int fail = 0;
    int sum = 0;
    int result = 0;
    for (int i = 0; i < num_of_workers; i++){
        previous_offset = Message_Read_from_one_socket(read_fds, i, &results, buffer, previous_offset, buffer_size);
        while(!Message_Is_End_Com(&results, 1)){
            result = atoi(results.args[0]);
            if (result < 0) // then query failed
                fail++;
            sum += atoi(results.args[0]);
            previous_offset = Message_Read_from_one_socket(read_fds, i, &results, buffer, previous_offset, buffer_size);
        }
    }
    Message_Destroy_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_Delete(&results);

    if (fail > 0){ // then something failed
        return -1;
    }
    else{ // everything was correct
        printf("%d\n", sum); fflush(stdout);
        
        // copy results to results message
        sprintf(results_string, "%d", sum);
        results_printed->num_of_args = 1;
        results_printed->args = malloc(sizeof(char*));
        results_printed->args[0] = malloc((strlen(results_string)+1)*sizeof(char));
        strcpy(results_printed->args[0], results_string);
        
        return 1;
    }
}

// server version 
// same but also they copy results to results messae
int topk_age_ranges_server(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, Message_vector* results_printed, int buffer_size){
    Message_Write_to_many(write_fds, num_of_workers, message, buffer_size);
// sleep(6);
    // init buffer to copy the printeed results to message results printed
    char results_string[64] = {0};
    Message_vector temp_result;
    Message_Init(&temp_result);

    // get results
    int *previous_offset;
    Buffer* buffer;
    Message_Create_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_vector results;
    Message_Init(&results);
    // read from every worker
    // end if worker has sent End of Communication
    int fail = 0;
    int sum = 0;
    int result = 0;
    int num_of_results = 0;
    for (int i = 0; i < num_of_workers; i++){
        previous_offset = Message_Read_from_one_socket(read_fds, i, &results, buffer, previous_offset, buffer_size);
        while(!Message_Is_End_Com(&results, 1)){
            result = atoi(results.args[0]);
            num_of_results = results.num_of_args;
            if(num_of_results == 1){
                if (result < 0) // then query failed
                    fail++;
            }
            else{ // something was found
                for (int i = 0; i < num_of_results; i+=2){
                    printf("%s: %s%% \n",results.args[i], results.args[i+1]);

                    // copy and append result to results printed message
                    memset(results_string, 0, 64);
                    sprintf(results_string, "%s: %s%% ",results.args[i], results.args[i+1] );
                    temp_result.num_of_args = 1;
                    temp_result.args = malloc(sizeof(char*));
                    temp_result.args[0] = malloc((strlen(results_string)+1)*sizeof(char));
                    strcpy(temp_result.args[0], results_string);
                    // append to results printed
                    // Message_Print(&temp_result);
                    Message_Append(results_printed, &temp_result);
                    // Message_Print(results_printed);
                    Message_Delete(&temp_result);
                }
            }
            previous_offset = Message_Read_from_one_socket(read_fds, i, &results, buffer, previous_offset, buffer_size);
        }
    }
    Message_Destroy_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_Delete(&results);

    if (fail > 0){ // then something failed
        return -1;
    }
    else{ // everything was correct
        return 1;
    }
}

// server version 
// same but also they copy results to results messae
int search_patient_record_server(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, Message_vector* results_printed, int buffer_size){
    Message_Write_to_many(write_fds, num_of_workers, message, buffer_size);

// sleep(6);
    // get results
    int *previous_offset;
    Buffer* buffer;
    Message_Create_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_vector results;
    Message_Init(&results);
    // read from every worker
    // end if worker has sent End of Communication
    int fail = 0;
    int sum = 0;
    int result = 0;
    int num_of_results = 0;
    for (int i = 0; i < num_of_workers; i++){
        previous_offset = Message_Read_from_one_socket(read_fds, i, &results, buffer, previous_offset, buffer_size);
        while(!Message_Is_End_Com(&results, 1)){
            result = atoi(results.args[0]);
            num_of_results = results.num_of_args;
            if(num_of_results == 1){
                if (result < 0) // then query failed
                    fail++;
            }
            else{ // something was found
                printf("%s %s %s %s %s %s %s\n",results.args[0], results.args[1], results.args[2], results.args[3], results.args[4], results.args[5], results.args[6]);

                // copy and append result to results printed message
                Message_Copy(results_printed, &results, 0, 0);
            }
            previous_offset = Message_Read_from_one_socket(read_fds, i, &results, buffer, previous_offset, buffer_size);
        }
    }
    Message_Destroy_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_Delete(&results);

    if (fail > 0){ // then something failed
        return -1;
    }
    else{ // everything was correct
        return 1;
    }
}

// server version 
// same but also they copy results to results messae
int num_patients_admissions_discharges_server(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, Message_vector* results_printed, int buffer_size){
    Message_Write_to_many(write_fds, num_of_workers, message, buffer_size);
// sleep(6);
    // init buffer to copy the printeed results to message results printed
    char results_string[64] = {0};
    Message_vector temp_result;
    Message_Init(&temp_result);

    // get results
    int *previous_offset;
    Buffer* buffer;
    Message_Create_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_vector results;
    Message_Init(&results);
    // read from every worker
    // end if worker has sent End of Communication
    int fail = 0;
    int sum = 0;
    int result = 0;
    int num_of_results = 0;
    for (int i = 0; i < num_of_workers; i++){
        previous_offset = Message_Read_from_one_socket(read_fds, i, &results, buffer, previous_offset, buffer_size);
        while(!Message_Is_End_Com(&results, 1)){
            result = atoi(results.args[0]);
            num_of_results = results.num_of_args;
            if(num_of_results == 1){
                if (result < 0) // then query failed
                    fail++;
            }
            else{ // something was found
                for (int i = 0; i < num_of_results; i+=2){
                    printf("%s %s\n",results.args[i], results.args[i+1]);

                    // copy and append result to results printed message
                    memset(results_string, 0, 64);
                    sprintf(results_string, "%s %s",results.args[i], results.args[i+1] );
                    temp_result.num_of_args = 1;
                    temp_result.args = malloc(sizeof(char*));
                    temp_result.args[0] = malloc((strlen(results_string)+1)*sizeof(char));
                    strcpy(temp_result.args[0], results_string);
                    // append to results printed
                    // Message_Print(&temp_result);
                    Message_Append(results_printed, &temp_result);
                    // Message_Print(results_printed);
                    Message_Delete(&temp_result);

                }
            }
            previous_offset = Message_Read_from_one_socket(read_fds, i, &results, buffer, previous_offset, buffer_size);
        }
    }
    Message_Destroy_buffer_offset(&buffer, &previous_offset, num_of_workers);
    Message_Delete(&results);

    if (fail > 0){ // then something failed
        return -1;
    }
    else{ // everything was correct
        return 1;
    }
}