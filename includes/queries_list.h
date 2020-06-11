#ifndef QUERIES_LIST_H
#define QUERIES_LIST_H


typedef struct Queries_List_node{

    char* query;
    struct Queries_List_node* next_node;
    
} Queries_List_node;

typedef struct Queries_List{

    int num_of_queries;
    Queries_List_node* first_node;
    Queries_List_node* last_node;

} Queries_List;


int QLIST_Init(Queries_List* list);
void QLIST_Destroy(Queries_List* list);

int QLIST_Insert(Queries_List* list, char* query);
char* QLIST_Get_Last_query(Queries_List* list);


#endif

