
#include "../includes/ht_avlt.h"
#include "../includes/patient_list.h"
#include "../includes/avl_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int powww(int b,int e){
    int base = 1;
    for (int i = 0; i < e; i++)
        base *= b;
    return base;   
}

unsigned int hash_function(Hash_Table* hash_table, char* string){
    int num_of_entries = hash_table->num_of_entries;
    unsigned int hash = 0;
    int p = 51;
    for (int i = 0; i < strlen(string); i++){
        hash += string[i]*powww(p,i);
    }
    return hash%num_of_entries;
}

Hash_Table_bucket * allocate_new_bucket(Hash_Table* hash_table){
    Hash_Table_bucket* new_bucket;
    int bucket_size = hash_table->bucket_size;
    new_bucket = malloc(sizeof(Hash_Table_bucket));
    new_bucket->next_bucket = NULL;
    new_bucket->data = malloc(bucket_size*sizeof(Hash_Table_data));
    for (int i = 0; i < bucket_size; i++)
        new_bucket->data[i].tree = NULL;
    new_bucket->records = 0;
    return new_bucket;
}

void HT_AVLT_Init(Hash_Table* hash_table, int num_of_entries, int bucket_size_bytes){

    hash_table->num_of_entries = num_of_entries;
    //calculate the real bucket size from bytes
    hash_table->bucket_size    = (bucket_size_bytes- (sizeof(Hash_Table_bucket) - sizeof(Hash_Table_data)))/sizeof(Hash_Table_data);
    // printf("%d - %d / %d = %d",bucket_size_bytes, sizeof(Hash_Table_bucket),  sizeof(Hash_Table_data), hash_table->bucket_size);
    hash_table->bucket = malloc(num_of_entries*sizeof(Hash_Table_bucket*));
    for (int i = 0; i < num_of_entries; i++){
        hash_table->bucket[i] = NULL;
    }
}


int HT_AVLT_Insert(Hash_Table* hash_table, char* string, Patient_List_node* patient){
    // hash function
    int index = hash_function(hash_table,string);
    int records;
    int bucket_size = hash_table->bucket_size;
    Hash_Table_bucket* bucket, *bucket_to_insert, *new_bucket, *bucket_found;
    Hash_Table_data* data;
    AVL_Tree* tree;
    // if null then first bucket of index is created
    if(hash_table->bucket[index] == NULL){
        hash_table->bucket[index] = allocate_new_bucket(hash_table);
    }
    
    //search if record does not exit instert it
    tree = HT_AVLT_Search(hash_table, string, &bucket_found);
    if( tree == NULL ){

        bucket_to_insert = bucket_found;
        while(bucket_to_insert->next_bucket != NULL)
            bucket_to_insert = bucket_to_insert->next_bucket;
        
        //inset data
        data    = bucket_to_insert->data;
        records = bucket_to_insert->records;
        //if records > bucket size allocate new bucket
        if(records >= hash_table->bucket_size){
            new_bucket = allocate_new_bucket(hash_table);
            bucket_to_insert->next_bucket = new_bucket;
            bucket_to_insert = new_bucket;
            data    = bucket_to_insert->data;
            records = bucket_to_insert->records;
        }

        //insert to AVL tree
        tree = data[records].tree;
        if(tree == NULL){
            data[records].tree = malloc(sizeof(AVL_Tree));
            AVLT_Init(data[records].tree);
            tree = data[records].tree;
        }
        AVLT_Insert(tree, patient);
        
        data[records].string = string;
        bucket_to_insert->records++;
        return 1;
    }
    else{
        AVLT_Insert(tree, patient);
    }

    // if exists do nothing an return 0
    return 0;
}

AVL_Tree * HT_AVLT_Search(Hash_Table* hash_table, char* string, Hash_Table_bucket** bucket_found){
    //hash function
    int index = hash_function(hash_table,string);
    int records;
    int bucket_size = hash_table->bucket_size;
    Hash_Table_bucket* bucket = hash_table->bucket[index];

    if(bucket == NULL)
        return NULL;
    
    while(bucket != NULL){
        *bucket_found = bucket;
        records = bucket->records;
        for (int i = 0; i < records; i++){
            if(strcmp(bucket->data[i].string, string) == 0)
                return bucket->data[i].tree; 
        }
        bucket = bucket->next_bucket;
    }

    return NULL;
}

void HT_AVLT_Print(Hash_Table* hash_table){

    int num_of_entries = hash_table->num_of_entries;
    int bucket_size    = hash_table->bucket_size;
    int records;
    Hash_Table_bucket* bucket_to_print, *bucket;

    for (int i = 0; i < num_of_entries; i++){
        bucket = hash_table->bucket[i];
        if(bucket != NULL){
            printf("\n\nbucket %d {",i);
            bucket_to_print = bucket;
            while(bucket_to_print != NULL){
                records = bucket_to_print->records;
                printf(" [");
                for (int j = 0; j < records; j++){
                    printf("%s, \n",bucket_to_print->data[j].string);
                    printf("<<><><><><><><>>\n");
                }
                printf("] ");
                bucket_to_print = bucket_to_print->next_bucket;
            }
            printf("}\n");
        }
    }
}


void HT_AVLT_Destroy(Hash_Table* hash_table){
    int num_of_entries = hash_table->num_of_entries;
    int bucket_size = hash_table->bucket_size;
    Hash_Table_bucket* bucket_to_delete, *bucket, *next_bucket_to_delete;
    AVL_Tree * tree;

    for (int i = 0; i < num_of_entries; i++){
        bucket = hash_table->bucket[i];
        if(bucket != NULL){
            bucket_to_delete = bucket;
            while(bucket_to_delete != NULL){
                next_bucket_to_delete = bucket_to_delete->next_bucket;
                //free avl trees
                for (int j = 0; j < bucket_size; j++){
                    tree = bucket_to_delete->data[j].tree;
                    if(tree != NULL){
                        AVLT_Destroy(tree);
                        free(tree);
                    }
                }
                free(bucket_to_delete->data);
                free(bucket_to_delete);
                bucket_to_delete = next_bucket_to_delete;
            }
        }
    }
    free(hash_table->bucket);
    
}



char* HT_AVLT_Get_key(Hash_Table* hash_table){
    int num_of_entries = hash_table->num_of_entries;
    int bucket_size    = hash_table->bucket_size;
    int records;

    static Hash_Table_bucket* bucket_to_print, *bucket;
    static int static_i = 0;
    static int static_j = 0; 
    static int next_bucket = 0;


    for (; static_i < num_of_entries; ){
        bucket = hash_table->bucket[static_i];
        if(bucket != NULL){
            if(next_bucket == 0)
                bucket_to_print = bucket;
            while(bucket_to_print != NULL){
                records = bucket_to_print->records;
                for (; static_j < records; static_j++){
                    static_j++;
                    return bucket_to_print->data[static_j-1].string;
                }
                static_j = 0;
                bucket_to_print = bucket_to_print->next_bucket;
                next_bucket = 1;
            }
        }
        static_i++;
        next_bucket = 0;
    }
    static_i = 0;
    static_j = 0;
    next_bucket = 0;
    return NULL;
}
