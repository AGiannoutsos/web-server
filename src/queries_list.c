#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/queries_list.h"

int QLIST_Init(Queries_List* list){
    memset(list, 0, sizeof(Queries_List));
    list->first_node = NULL;
    list->last_node = NULL;
}



int QLIST_Insert(Queries_List* list, char* query){
    list->num_of_queries++;
    Queries_List_node* new_node;

    // allocate new node
    new_node = malloc(sizeof(Queries_List_node));
    memset(new_node, 0 , sizeof(Queries_List_node));

    // allocate memeory for query
    new_node->query = malloc((strlen(query)+1)*sizeof(char));
    strcpy(new_node->query, query);
    new_node->next_node = NULL;


    // if last node null ther add first element
    if (list->last_node == NULL){
        list->first_node = new_node;
        list->last_node = new_node;
    }
    else{
        list->last_node->next_node = new_node;
        list->last_node = new_node;
    }
  

}

void QLIST_Destroy(Queries_List* list){
    Queries_List_node* node_to_delete = list->first_node;
    Queries_List_node* next_node;
    
    while(node_to_delete != NULL){
        next_node = node_to_delete->next_node;
        free(node_to_delete->query);
        free(node_to_delete);
        node_to_delete = next_node;
    }
}

char * QLIST_Get_Last_query(Queries_List* list){
    return list->last_node->query;
}