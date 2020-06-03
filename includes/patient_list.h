#ifndef PATIENT_LIST_H
#define PATIENT_LIST_H

#include "date.h"
#include "hashtable.h"

#define WORD_SIZE 64

typedef struct Patient_List_node{

    char record_id[WORD_SIZE];
    char fisrt_name[WORD_SIZE];
    char last_name[WORD_SIZE];
    char disease_id[WORD_SIZE];
    char country[WORD_SIZE];
    char age[WORD_SIZE];


    Date entry_date;
    char valid_exit_date;
    Date exit_date;

    struct Patient_List_node* next_node;

} Patient_List_node;


typedef struct Patient_List{

    struct Patient_List_node* first_node;
    struct Patient_List_node* last_node;
    Hash_Table_s record_id_hash_table;

} Patient_List;

void PL_Init(Patient_List* list);

int PL_Insert(Patient_List* list, char* record);
void PL_Print(Patient_List* list);
void print_node(Patient_List_node* node);
void PL_Destroy(Patient_List* list);
Patient_List_node * PL_SearchID(Patient_List* list, char* id);
Patient_List_node * PL_Get_Last_node(Patient_List* list);

#endif