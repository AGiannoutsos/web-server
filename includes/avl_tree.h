#ifndef AVL_TREE_H
#define AVL_TREE_H
#include "date.h"
#include "patient_list.h"


typedef struct Duplicate_Date_List{
    Patient_List_node * patient;
    struct Duplicate_Date_List* next_node;
}Duplicate_Date_List;


typedef struct AVL_Tree_node{
    
    int depth;
    int num_of_patients;
    Patient_List_node * patient;
    struct AVL_Tree_node * left_node;
    struct AVL_Tree_node * right_node;
    Duplicate_Date_List * doublicate_date_list_first_node;
    Duplicate_Date_List * doublicate_date_list_last_node;

} AVL_Tree_node;

typedef struct AVL_Tree{

    int nodes;
    AVL_Tree_node *first_node;

} AVL_Tree;



void AVLT_Init(AVL_Tree* tree);
void AVLT_Destroy(AVL_Tree* tree);
void AVLT_Print(AVL_Tree* tree);
AVL_Tree_node * AVLT_Print_r(AVL_Tree_node* tree_node);
int AVLT_Date_compare(Patient_List_node* patient1, Patient_List_node* patient2);
AVL_Tree_node * AVLT_Insert(AVL_Tree* tree, Patient_List_node* patient);

// Get the num of records in a avl tree having no exit date
int AVLT_Records_no_exit(AVL_Tree* tree);
int AVLT_Records_no_exit_r(AVL_Tree_node* tree_node);

// Get the num of records in a avl tree between these dates
int AVLT_Records_between(AVL_Tree* tree, Date* date1, Date* date2);
int AVLT_Records_between_r(AVL_Tree_node* tree_node, Date* date1, Date* date2);

// Get the num of records in a avl tree between these dates and the given country
int AVLT_Join_product_countries(AVL_Tree* tree, Date* date1, Date* date2, char* country);
int AVLT_Join_product_countries_r(AVL_Tree_node* tree, Date* date1, Date* date2, char* country);

// Get the num of records in a avl tree between these exit dates and the given country
int AVLT_Join_product_countries_exit_date(AVL_Tree* tree, Date* date1, Date* date2, char* country);
int AVLT_Join_product_countries_exit_date_r(AVL_Tree_node* tree, Date* date1, Date* date2, char* country);

// Get the num of records in a avl tree between these dates and the given disease
int AVLT_Join_product_diseases(AVL_Tree* tree, Date* date1, Date* date2, char* disease);
int AVLT_Join_product_diseases_r(AVL_Tree_node* tree, Date* date1, Date* date2, char* disease);

// Get the num of records in a avl tree between these dates and between the given ages
int AVLT_Join_product_ages(AVL_Tree* tree, Date* date1, Date* date2, int age1, int age2);
int AVLT_Join_product_ages_r(AVL_Tree_node* tree_node, Date* date1, Date* date2, int age1, int age2);

// Get the num of records in a avl tree between these dates and the given disease and between the given ages
int AVLT_Join_product_ages_diseases_r(AVL_Tree_node* tree_node, Date* date1, Date* date2, char* disease, int age1, int age2);
int AVLT_Join_product_ages_diseases(AVL_Tree* tree, Date* date1, Date* date2, char* disease, int age1, int age2);

#endif