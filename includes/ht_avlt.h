#ifndef HT_AVLT_H
#define HT_AVLT_H

#include "avl_tree.h"
#include "patient_list.h"


typedef struct Hash_Table_data{
    char* string;
    AVL_Tree * tree;
}Hash_Table_data;

typedef struct Hash_Table_bucket{

    int records;
    Hash_Table_data * data;
    struct Hash_Table_bucket * next_bucket;

}Hash_Table_bucket;

typedef struct Hash_Table{

    int num_of_entries;
    int bucket_size;
    Hash_Table_bucket** bucket;

} Hash_Table;


void HT_AVLT_Init(Hash_Table* hash_table, int num_of_entries, int bucket_size_bytes);
void HT_AVLT_Print(Hash_Table* hash_table);
void HT_AVLT_Destroy(Hash_Table* hash_table);

AVL_Tree *  HT_AVLT_Search(Hash_Table* hash_table, char* string, Hash_Table_bucket** bucket_found);
int HT_AVLT_Insert(Hash_Table* hash_table, char* string, Patient_List_node* patient);

char* HT_AVLT_Get_key(Hash_Table* hash_table);

#endif