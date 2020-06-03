#ifndef QUERIES_WORKER_H
#define QUERIES_WORKER_H

#include "../includes/hashtable.h"
#include "../includes/patient_list.h"
#include "../includes/avl_tree.h"
#include "../includes/ht_avlt.h"



int disease_frequency(Message_vector* results, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table);
int write_disease_frequency(int write_fd, int buffer_size, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table);

int topk_age_ranges(Message_vector* results, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table);
int write_topk_age_ranges(int write_fd, int buffer_size, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table);

int search_patient_record(Message_vector* results, int num_of_args, char** args, Patient_List* patient_list);
int write_search_patient_record(int write_fd, int buffer_size, int num_of_args, char** args, Patient_List* patient_list);

int num_patient_admissions(Message_vector* results, Worker* me_worker, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table);
int write_num_patient_admissions(int write_fd, int buffer_size, Worker* me_worker, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table);

int num_patient_discharges(Message_vector* results, Worker* me_worker, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table);
int write_num_patient_discharges(int write_fd, int buffer_size, Worker* me_worker, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table);



#endif
