#include "../includes/patient_list.h"
#include "../includes/date.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void PL_Init(Patient_List* list){
    list->first_node = NULL;
    list->last_node  = NULL;
    HT_Init_simple(&(list->record_id_hash_table),50,512);
}
int valid_record_id(Patient_List* list, Patient_List_node* node){
    //chech if record id is used again

    if(HT_Insert_simple(&(list->record_id_hash_table), node->record_id) > 0){
        return 1;
    }
    else{
        // printf("\n\n***ERROR record id already inserted***\n***Shut down***\n\n");
        // printf("error\n");
        // sleep(1);
        return -1;
    }
}

void destroy_node(Patient_List_node *node){
    free(node->record_id);
    free(node->fisrt_name);
    free(node->last_name);
    free(node->disease_id);
    free(node->country);
    free(node->age);
    free(node);
}

char* new_word(char* string){
    char* new_word = malloc((strlen(string)+1)*sizeof(char));
    memset(new_word, 0, (strlen(string+1))*sizeof(char));
    strcpy(new_word,string);
    return new_word;
}

int insert_record(Patient_List_node* node_to_insert,char* record){
    char* word_buf;
    word_buf = strtok(record," ");
    // node_to_insert->entry_date.day = 0;

    // node_to_insert->record_id = malloc((40+1)*sizeof(char));
    // memset(node_to_insert->record_id, 0, 41);
    strcpy(node_to_insert->record_id, word_buf);

    // node_to_insert->record_id = new_word(word_buf);

    word_buf = strtok(NULL," ");
    // node_to_insert->fisrt_name = malloc((40+1)*sizeof(char));
    // memset(node_to_insert->fisrt_name, 0, 41);
    strcpy(node_to_insert->fisrt_name, word_buf);

    // node_to_insert->fisrt_name = new_word(word_buf);

    word_buf = strtok(NULL," ");
    // node_to_insert->last_name = malloc((40+1)*sizeof(char));
    // memset(node_to_insert->last_name, 0, 41);
    strcpy(node_to_insert->last_name, word_buf);

    // node_to_insert->last_name = new_word(word_buf);

    word_buf = strtok(NULL," ");
    // node_to_insert->disease_id = malloc((40+1)*sizeof(char));
    // memset(node_to_insert->disease_id, 0, 41);
    strcpy(node_to_insert->disease_id, word_buf);

    // node_to_insert->disease_id = new_word(word_buf);

    word_buf = strtok(NULL," ");
    // node_to_insert->country = malloc((40+1)*sizeof(char));
    // memset(node_to_insert->country, 0, 41);
    strcpy(node_to_insert->country, word_buf);

    word_buf = strtok(NULL," ");
    // node_to_insert->age = malloc((40+1)*sizeof(char));
    // memset(node_to_insert->age, 0, 41);
    strcpy(node_to_insert->age, word_buf);

    // node_to_insert->country = new_word(word_buf);

    word_buf = strtok(NULL," ");
    DATE_Insert(&(node_to_insert->entry_date), word_buf);

    //check for exit date
    word_buf = strtok(NULL," ");
    if(strcmp(word_buf,"-\n") == 0 || strcmp(word_buf,"-") == 0){
        node_to_insert->valid_exit_date = 0;
    }
    else{
        node_to_insert->valid_exit_date = 1;
        DATE_Insert(&(node_to_insert->exit_date), word_buf);
        //check if dates are correct
        if( DATE_Compare(&(node_to_insert->entry_date), &(node_to_insert->exit_date)) < 0){
            return 1;
        }
        else{
            // printf("error\n");
            // printf("\n\n***ERROR wrong date order (%s)***\n***Record not inserted***\n\n",node_to_insert->record_id);
            return 0;
        }
    }

    return 1;
}

int PL_Insert(Patient_List* list,char* record){
    //allocate for new node
    Patient_List_node* node_to_insert = malloc(sizeof(Patient_List_node));
    memset(node_to_insert, 0, sizeof(Patient_List_node));
    //insert record in patiend node
    node_to_insert->next_node = NULL;
    //if record valid then place in in the list or delete it
    if(insert_record(node_to_insert,record) > 0){
        //check if record id is not used
        if(valid_record_id(list,node_to_insert) < 0){
            // destroy_node(node_to_insert);

            free(node_to_insert);
            //wrong id return -1
            return -1;
        }

        //check if first node is null
        if(list->first_node == NULL){
            //allocate first node
            list->first_node = node_to_insert;
            list->last_node = list->first_node;
        }
        else{
            list->last_node->next_node = node_to_insert;
            list->last_node = node_to_insert;
        }
        // correct
        return 1;
    }
    else{

        free(node_to_insert);
        //wrong date return 0
        return 0;
    }

}

void print_node(Patient_List_node* node){
    printf("id: %s\n",node->record_id);
    printf("first name: %s\n",node->fisrt_name);
    printf("last name: %s\n",node->last_name);
    printf("disease: %s\n",node->disease_id);   
    printf("country: %s\n",node->country);
    printf("age: %s\n",node->age);
    printf("entry date ");
    DATE_Print(&(node->entry_date));
    printf("exit date ");
    if(node->valid_exit_date == 1)
        DATE_Print(&(node->exit_date));
    else
        printf("no exit date\n");
}


void PL_Print(Patient_List* list){
    Patient_List_node* node_to_print = list->first_node;

    while(node_to_print != NULL){
        printf("--------------------\n");
        print_node(node_to_print);
        node_to_print = node_to_print->next_node;
        printf("--------------------\n");
    }



}



void PL_Destroy(Patient_List* list){
    Patient_List_node* node_to_delete = list->first_node;
    Patient_List_node* next_node;
    HT_Destroy_simple(&(list->record_id_hash_table));
    
    while(node_to_delete != NULL){
        next_node = node_to_delete->next_node;
        free(node_to_delete);
        node_to_delete = next_node;
    }
}

Patient_List_node * PL_Get_Last_node(Patient_List* list){
    return list->last_node;
}

Patient_List_node * PL_SearchID(Patient_List* list, char* id){
    Patient_List_node* curr_node = list->first_node;
    
    while(curr_node != NULL){
        if(strcmp(curr_node->record_id, id) == 0)
            return curr_node;
        curr_node = curr_node->next_node;
    }
}