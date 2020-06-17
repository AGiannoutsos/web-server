#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/select.h>

#include "../includes/message_handlers.h"
#include "../includes/util.h"

#include "../includes/hashtable.h"
#include "../includes/patient_list.h"
#include "../includes/avl_tree.h"
#include "../includes/ht_avlt.h"
#include "../includes/queries_worker.h"


int Data_Structures_Insertion(char* file_buf, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table){
    int valid_insertion = 0;
    Patient_List_node* last_node = NULL;
    if(strlen(file_buf) > 5)
        valid_insertion = PL_Insert(patient_list,file_buf);
        
        //fill the disease and country hash tables
        if(valid_insertion > 0){
            last_node = PL_Get_Last_node(patient_list);
            //insert disease hash table
            HT_AVLT_Insert(disease_hash_table, last_node->disease_id, last_node);
            
            //insert country hash table
            HT_AVLT_Insert(country_hash_table, last_node->country, last_node);

            return 1;
        }
        // wrong date
        else if(valid_insertion == 0)
            return 0;
        else
        // wrong record id Shut down the process
            return -1;
}

int Update_patient_exit(char* id, char* exit_date, Patient_List* patient_list){

    // update the exit date, search the patient list for the id
    // if there is no patient id do nothing
    Patient_List_node* patient_list_node = PL_SearchID(patient_list,id);
    Date d1;
    if(patient_list_node != NULL){
        //check if exit date is correct
        DATE_Insert(&d1, exit_date);
        if(DATE_Compare(&(patient_list_node->entry_date), &d1) <= 0 ){
            patient_list_node->valid_exit_date = 1;
            DATE_Insert(&(patient_list_node->exit_date), exit_date);
            // everything correct return 1
            return 1;
        }
        else{
            // error exit date was before entry
            return -1;
        }
        // PL_Print(patient_list);
    }
    else
        // error id was not found
        return -2;
}

int get_files_in_data_structures_send_statistics(int write_fd, int buffer_size, char* input_dir, char** directories, int num_of_directories, Hash_Table_s* files_hashtable, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table){
    char path[READ_BUFFER_SIZE] = {0};
    char read_file_buffer[READ_BUFFER_SIZE] = {0};
    char record_buffer[READ_BUFFER_SIZE] = {0};

    Hash_Table_bucket_s null_bucket;
    FILE* file_fd;
    int exist_status = 1;
    int enter_status = 1;
    int exit_status_ = 1;
    int content_status = 1;

    Message_vector temp_files;
    Message_Init(&temp_files);
    Message_vector file_content;
    Message_Init(&file_content);
    Message_vector statistics;
    Message_Init(&statistics);

    // for every directory
    for (int directory = 0; directory < num_of_directories; directory++){

        sprintf(path, "%s/%s",input_dir, directories[directory]);
        // printf("p> %s\n",path);
        // get files from the path
        get_directory_content(path, DT_REG, &(temp_files.num_of_args), &(temp_files.args));

        // sort files in the directory
        qsort(temp_files.args, temp_files.num_of_args, sizeof(char*), DATE_Compare_string);

        // insert files to hashtable
        for (int file = 0; file < temp_files.num_of_args; file++){
            // if the file is already inserted then dont add it
            // this can be used if a new file is added in thw directory
            // use full path of file
            memset(path, 0, READ_BUFFER_SIZE);
            sprintf(path, "%s/%s/%s",input_dir, directories[directory], temp_files.args[file]);
            exist_status = HT_Insert_simple(files_hashtable, path);

            // if exist_status = 1 then file is new so it needs to update the data structures
            // update data structures
            if (exist_status == 1){

                // get the file content and transorm them to a string
                // open file
                file_fd = fopen(path, "r");
                if (file_fd < 0)
                    perror("open file");
                    // read every record in the file
                // init statistics per file
                Message_Delete(&statistics);
                Message_Init(&statistics);
                
                // read every record in file
                fgets(read_file_buffer, READ_BUFFER_SIZE, file_fd);
                while( !feof(file_fd) ){

                    if (read_file_buffer[0] == '\n') break;

                    Message_to_vector(read_file_buffer, &file_content);
                    // no content error
                    if( file_content.num_of_args < 6)
                        content_status = 0;
                        
                    if (content_status){
                        sprintf(record_buffer, "%s %s %s %s %s %s %s -", file_content.args[0], file_content.args[2], file_content.args[3], file_content.args[4],  directories[directory], file_content.args[5], temp_files.args[file]);

                        // check for errors in the records
                        // if record is enter then add it to data structures
                        if (strcmp(file_content.args[1], "ENTER") == 0){

                            enter_status = Data_Structures_Insertion(record_buffer, patient_list, disease_hash_table, country_hash_table);
                            // if enter was correct then update statistics
                            if (enter_status == 1){
                                Statistics_Update(&statistics, &file_content, directories[directory], temp_files.args[file]);
                            }

                        }
                        // else if record  is exit then update the datastructures exit date
                        else{
                            exit_status_ = Update_patient_exit(file_content.args[0], temp_files.args[file], patient_list);
                        }
                    }

                    // if something went wrong print error
                    if(enter_status <= 0 || exit_status_ <= 0 || content_status <= 0){
                        fprintf(stderr, "ERROR\n");  fflush(stdout);
                    }

                    // reset counters
                    content_status = 1;
                    enter_status = 1;
                    exit_status_ = 1;
                    memset(read_file_buffer, 0, READ_BUFFER_SIZE);
                    memset(record_buffer, 0, READ_BUFFER_SIZE);
                    fgets(read_file_buffer, READ_BUFFER_SIZE, file_fd);
                    Message_Delete(&file_content);
                }
                fclose(file_fd);
                
                // send the statistics to aggragator
                Message_Write(write_fd, &statistics, buffer_size);
                // Statistics_Print(&statistics, stderr);

            } // if file doenst exist
        } // every file
        
        // reset variables
        Message_Delete(&temp_files);
        memset(path, 0, READ_BUFFER_SIZE);

    } // every directory   

    Message_Delete(&statistics);
    // end communication with aggragator
    Message_Write_End_Com(write_fd, buffer_size); 
}



void print_log(Worker* me_worker, int success, int fail){
    // create and print logs to log file
    char log_file_name[32] = {0};
    sprintf(log_file_name, "log_file.%d", getpid());
    FILE* log_file = fopen(log_file_name, "w");

    for (int i = 0; i < me_worker->num_of_directories; i++){
        fprintf(log_file, "%s\n", me_worker->directories[i]);
    }
    fprintf(log_file, "TOTAL %d\n", success+fail);
    fprintf(log_file, "SUCCESS %d\n", success);
    fprintf(log_file, "FAIL %d\n", fail);
    fclose(log_file);
}

void free_worker(Worker** me_worker, int** write_fd, int** read_fd, Buffer** buffer, int** previous_offset, int pipe_id, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table, Hash_Table_s* files_hashtable){
    HT_Destroy_simple(files_hashtable);
    HT_AVLT_Destroy(country_hash_table);
    HT_AVLT_Destroy(disease_hash_table);
    PL_Destroy(patient_list);
    // Message_Destroy_buffer_offset(buffer, previous_offset, 1);
    close_pipes(write_fd, "master", &pipe_id, 1);
    close_pipes(read_fd, "worker", &pipe_id, 1);
    Worker_Destroy(me_worker, 1);
}



