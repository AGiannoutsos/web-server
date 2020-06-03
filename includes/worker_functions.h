#ifndef WORKER_FUNCTIONS_H
#define WORKER_FUNCTIONS_H

#include "../includes/message_handlers.h"
#include "../includes/util.h"

#include "../includes/hashtable.h"
#include "../includes/patient_list.h"
#include "../includes/avl_tree.h"
#include "../includes/ht_avlt.h"

int Data_Structures_Insertion(char* file_buf, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table);
int Update_patient_exit(char* id, char* exit_date, Patient_List* patient_list);
int get_files_in_data_structures_send_statistics(int write_fd, int buffer_size, char* input_dir, char** directories, int num_of_directories, Hash_Table_s* files_hashtable, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table);
void print_log(Worker* me_worker, int success, int fail);
void free_worker(Worker** me_worker, int** write_fd, int** read_fd, Buffer** buffer, int** previous_offset, int pipe_id, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table, Hash_Table_s* files_hashtable);
void free_worker(Worker** me_worker, int** write_fd, int** read_fd, Buffer** buffer, int** previous_offset, int pipe_id, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table, Hash_Table_s* files_hashtable);

#endif