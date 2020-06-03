#ifndef HASHTABLE_H
#define HASHTABLE_H


typedef struct Hash_Table_data_s{
    char* string;
}Hash_Table_data_s;

typedef struct Hash_Table_bucket_s{

    int records;
    Hash_Table_data_s * data;
    struct Hash_Table_bucket_s * next_bucket;

}Hash_Table_bucket_s;

typedef struct Hash_Table_s{

    int total_records;
    int num_of_entries;
    int bucket_size;
    Hash_Table_bucket_s** bucket;

} Hash_Table_s;


void HT_Init_simple(Hash_Table_s* hash_table, int num_of_entries, int bucket_size_bytes);
int HT_Insert_simple(Hash_Table_s* hash_table, char* data);
int HT_Search_simple(Hash_Table_s* hash_table, char* data, Hash_Table_bucket_s** bucket_found);
void HT_Print_simple(Hash_Table_s* hash_table);
void HT_Destroy_simple(Hash_Table_s* hash_table);
char* HT_Get_key(Hash_Table_s* hash_table);

#endif