#include "../includes/avl_tree.h"
// #include "patient_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void AVLT_Init(AVL_Tree* tree){
    tree->nodes = 0;
    tree->first_node = NULL;
}

int AVLT_Date_compare(Patient_List_node* patient1, Patient_List_node* patient2){
    return DATE_Compare(&(patient1->entry_date), &(patient2->entry_date));
}

int depth(AVL_Tree_node* node){
    if(node == NULL)
        return 0;
    else
        return node->depth;
}

int max_depth(AVL_Tree_node* left, AVL_Tree_node* right){
    int depth1,depth2;
    
    depth1 = depth(left);
    depth2 = depth(right);

    if (depth1 > depth2)
        return depth1;
    return depth2;
}

AVL_Tree_node * allocate_new_tree_node(Patient_List_node* patient){
    AVL_Tree_node * new_node = malloc(sizeof(AVL_Tree_node));
    memset(new_node, 0, sizeof(AVL_Tree_node));
    new_node->left_node  = NULL;
    new_node->right_node = NULL;
    new_node->doublicate_date_list_first_node = NULL;
    new_node->doublicate_date_list_last_node  = NULL;
    new_node->depth      = 1;
    new_node->patient    = patient;
    new_node->num_of_patients++;
    return new_node;
}

void insert_doublicate(AVL_Tree_node* tree_node, Patient_List_node* patient){
    Duplicate_Date_List* new_node;
    if(tree_node->doublicate_date_list_first_node == NULL){
        new_node = malloc(sizeof(Duplicate_Date_List));
        new_node->next_node = NULL;
        new_node->patient = patient;
        tree_node->doublicate_date_list_first_node = new_node;
        tree_node->doublicate_date_list_last_node  = new_node;
        tree_node->num_of_patients++;
        return;
    }

    Duplicate_Date_List* temp_node = tree_node->doublicate_date_list_last_node;
    new_node = malloc(sizeof(Duplicate_Date_List));
    new_node->next_node = NULL;
    new_node->patient = patient;
    temp_node->next_node = new_node;
    tree_node->num_of_patients++;
    tree_node->doublicate_date_list_last_node  = new_node;
    return;
        
}

void destroy_douplicates(AVL_Tree_node* tree_node){
    Duplicate_Date_List* next_node = tree_node->doublicate_date_list_first_node;
    Duplicate_Date_List* temp_node;
    while(next_node != NULL){
        temp_node = next_node->next_node;
        free(next_node);
        next_node = temp_node;
    }
}


AVL_Tree_node * AVLT_Print_r(AVL_Tree_node* tree_node){
    
    if(tree_node == NULL){
        return NULL;
    }

    AVLT_Print_r(tree_node->left_node);
    printf("-----------------\n");
    print_node(tree_node->patient);
    AVLT_Print_r(tree_node->right_node);

}

void AVLT_Print(AVL_Tree* tree){
    AVLT_Print_r(tree->first_node);
}


int AVLT_Records_no_exit_r(AVL_Tree_node* tree_node){
    
    if(tree_node == NULL){
        return 0;
    }
    int records_right = 0;
    int records_left  = 0;
    int records_node_has = 0;

    // Handle duplicates
    if (tree_node->patient->valid_exit_date == 0){
        records_node_has++;
    }

    Duplicate_Date_List *next_dublicate = tree_node->doublicate_date_list_first_node;
    while(next_dublicate != NULL){
        if (next_dublicate->patient->valid_exit_date == 0){
            records_node_has++;
        }
        next_dublicate = next_dublicate->next_node;
    }

    // search both sides for records
    records_left = AVLT_Records_no_exit_r(tree_node->left_node);
    records_right = AVLT_Records_no_exit_r(tree_node->right_node);

    // return the sum of the results
    return records_left + records_right + records_node_has;

}
int AVLT_Records_no_exit(AVL_Tree* tree){
    return AVLT_Records_no_exit_r(tree->first_node);
}


int AVLT_Records_between_r(AVL_Tree_node* tree_node, Date* date1, Date* date2){
    
    if(tree_node == NULL){
        return 0;
    }
    Date patient_date = tree_node->patient->entry_date;
    int records_right = 0;
    int records_left  = 0;
    int num_of_patients = tree_node->num_of_patients;
    int compare1 = DATE_Compare(&patient_date,date1);
    int compare2 = DATE_Compare(&patient_date,date2);
    //if patient is less or equal than the date1 then there is no point to search left node
    if (compare1 > 0)
        records_left = AVLT_Records_between_r(tree_node->left_node,date1,date2);

    //if patient is greater or equal than the date2 then there is no point to search right node
    if(compare2 < 0)
        records_right = AVLT_Records_between_r(tree_node->right_node,date1,date2);
    //if current node date is between the dates then return left+right +1
    if( compare1 >= 0  &&   compare2 <= 0){
        // printf("date-> "); DATE_Print(&patient_date);
        return records_left + records_right + num_of_patients;
    }
    return records_left + records_right;
}
int AVLT_Records_between(AVL_Tree* tree, Date* date1, Date* date2){
    return AVLT_Records_between_r(tree->first_node,date1,date2);
}

int between(int age, int age1, int age2){
    if(age>=age1 && age<=age2)
        return 1;
    else
        return 0;
}

int AVLT_Join_product_ages_r(AVL_Tree_node* tree_node, Date* date1, Date* date2, int age1, int age2){
    
    if(tree_node == NULL){
        return 0;
    }
    Date* patient_date = &(tree_node->patient->entry_date);
    int patient_age = atoi(tree_node->patient->age);
    int records_right = 0;
    int records_left  = 0;
    // int num_of_patients = tree_node->num_of_patients;
    int compare1;
    int compare2;
    int node_has_age = 0;

    if ( between(patient_age, age1, age2) ){
        node_has_age++;
    }

    Duplicate_Date_List *next_dublicate = tree_node->doublicate_date_list_first_node;
    while(next_dublicate != NULL){
        if ( between(atoi(next_dublicate->patient->age), age1, age2) ){
            node_has_age++;
        }
        next_dublicate = next_dublicate->next_node;
    }

    //date is conditional
    if(!(date1 == NULL || date2 == NULL)){
        compare1 = DATE_Compare(patient_date,date1);
        compare2 = DATE_Compare(patient_date,date2);
    }
    else{
        compare1 = 1;
        compare2 = -1;
    }
    //if patient is less or equal than the date1 then there is no point to search left node
    if (compare1 > 0)
        records_left = AVLT_Join_product_ages_r(tree_node->left_node,date1,date2, age1, age2);

    //if patient is greater or equal than the date2 then there is no point to search right node
    if(compare2 < 0)
        records_right = AVLT_Join_product_ages_r(tree_node->right_node,date1,date2, age1, age2);
    //if current node date is between the dates then return left+right +1
    // AND if they have the same country as the query
    if( compare1 >= 0  &&  compare2 <= 0  &&  node_has_age ){
        // printf("date->(%d) ",node_has_country); DATE_Print(patient_date);
        return records_left + records_right + node_has_age;
    }
    return records_left + records_right;
}

int AVLT_Join_product_ages(AVL_Tree* tree, Date* date1, Date* date2, int age1, int age2){
    return AVLT_Join_product_ages_r(tree->first_node,date1,date2, age1, age2);
}

int AVLT_Join_product_countries_exit_date_r(AVL_Tree_node* tree_node, Date* date1, Date* date2, char* country){
    
    if(tree_node == NULL){
        return 0;
    }
    Date* patient_date;
    //  = &(tree_node->patient->entry_date);
    // if has valid exit date then get the exit date
    if(tree_node->patient->valid_exit_date == 1){
        patient_date = &(tree_node->patient->entry_date);
    }
    else{ // else get a 0-0-0 date
        Date null_date;
        null_date.day = 0;
        null_date.month = 0;
        null_date.year = 0;
        patient_date = &null_date;
    }

    char* patient_country = tree_node->patient->country;
    int records_right = 0;
    int records_left  = 0;
    // int num_of_patients = tree_node->num_of_patients;
    int compare1;
    int compare2;
    int node_has_country = 0;

    if (strcmp(patient_country, country) == 0){
        node_has_country++;
    }

    Duplicate_Date_List *next_dublicate = tree_node->doublicate_date_list_first_node;
    while(next_dublicate != NULL){
        if (strcmp(next_dublicate->patient->country, country) == 0){
            node_has_country++;
        }
        next_dublicate = next_dublicate->next_node;
    }

    //date is conditional
    if(!(date1 == NULL || date2 == NULL)){
        compare1 = DATE_Compare(patient_date,date1);
        compare2 = DATE_Compare(patient_date,date2);
    }
    else{
        compare1 = 1;
        compare2 = -1;
    }
    //if patient is less or equal than the date1 then there is no point to search left node
    if (compare1 > 0)
        records_left = AVLT_Join_product_countries_exit_date_r(tree_node->left_node,date1,date2, country);

    //if patient is greater or equal than the date2 then there is no point to search right node
    if(compare2 < 0)
        records_right = AVLT_Join_product_countries_exit_date_r(tree_node->right_node,date1,date2, country);
    //if current node date is between the dates then return left+right +1
    // AND if they have the same country as the query
    if( compare1 >= 0  &&  compare2 <= 0  &&  node_has_country ){
        // printf("date->(%d) ",node_has_country); DATE_Print(patient_date);
        return records_left + records_right + node_has_country;
    }
    return records_left + records_right;
}

int AVLT_Join_product_countries_exit_date(AVL_Tree* tree, Date* date1, Date* date2, char* country){
    return AVLT_Join_product_countries_exit_date_r(tree->first_node,date1,date2, country);
}


int AVLT_Join_product_countries_r(AVL_Tree_node* tree_node, Date* date1, Date* date2, char* country){
    
    if(tree_node == NULL){
        return 0;
    }
    Date* patient_date = &(tree_node->patient->entry_date);
    char* patient_country = tree_node->patient->country;
    int records_right = 0;
    int records_left  = 0;
    // int num_of_patients = tree_node->num_of_patients;
    int compare1;
    int compare2;
    int node_has_country = 0;

    if (strcmp(patient_country, country) == 0){
        node_has_country++;
    }

    Duplicate_Date_List *next_dublicate = tree_node->doublicate_date_list_first_node;
    while(next_dublicate != NULL){
        if (strcmp(next_dublicate->patient->country, country) == 0){
            node_has_country++;
        }
        next_dublicate = next_dublicate->next_node;
    }

    //date is conditional
    if(!(date1 == NULL || date2 == NULL)){
        compare1 = DATE_Compare(patient_date,date1);
        compare2 = DATE_Compare(patient_date,date2);
    }
    else{
        compare1 = 1;
        compare2 = -1;
    }
    //if patient is less or equal than the date1 then there is no point to search left node
    if (compare1 > 0)
        records_left = AVLT_Join_product_countries_r(tree_node->left_node,date1,date2, country);

    //if patient is greater or equal than the date2 then there is no point to search right node
    if(compare2 < 0)
        records_right = AVLT_Join_product_countries_r(tree_node->right_node,date1,date2, country);
    //if current node date is between the dates then return left+right +1
    // AND if they have the same country as the query
    if( compare1 >= 0  &&  compare2 <= 0  &&  node_has_country ){
        // printf("date->(%d) ",node_has_country); DATE_Print(patient_date);
        return records_left + records_right + node_has_country;
    }
    return records_left + records_right;
}

int AVLT_Join_product_countries(AVL_Tree* tree, Date* date1, Date* date2, char* country){
    return AVLT_Join_product_countries_r(tree->first_node,date1,date2, country);
}


int AVLT_Join_product_diseases_r(AVL_Tree_node* tree_node, Date* date1, Date* date2, char* disease){
    
    if(tree_node == NULL){
        return 0;
    }
    Date* patient_date = &(tree_node->patient->entry_date);
    char* patient_disease = tree_node->patient->disease_id;
    int records_right = 0;
    int records_left  = 0;
    // int num_of_patients = tree_node->num_of_patients;
    int compare1;
    int compare2;
    int node_has_disease = 0;

    if (strcmp(patient_disease, disease) == 0){
        node_has_disease++;
    }

    Duplicate_Date_List *next_dublicate = tree_node->doublicate_date_list_first_node;
    while(next_dublicate != NULL){
        if (strcmp(next_dublicate->patient->disease_id, disease) == 0){
            node_has_disease++;
        }
        next_dublicate = next_dublicate->next_node;
    }

    //date is conditional
    if(!(date1 == NULL || date2 == NULL)){
        compare1 = DATE_Compare(patient_date,date1);
        compare2 = DATE_Compare(patient_date,date2);
    }
    else{
        compare1 = 1;
        compare2 = -1;
    }
    //if patient is less or equal than the date1 then there is no point to search left node
    if (compare1 > 0)
        records_left = AVLT_Join_product_diseases_r(tree_node->left_node,date1,date2, disease);

    //if patient is greater or equal than the date2 then there is no point to search right node
    if(compare2 < 0)
        records_right = AVLT_Join_product_diseases_r(tree_node->right_node,date1,date2, disease);
    //if current node date is between the dates then return left+right +1
    // AND if they have the same disease as the query
    if( compare1 >= 0  &&  compare2 <= 0  &&  node_has_disease ){
        // printf("date->(%d) ",node_has_country); DATE_Print(patient_date);
        return records_left + records_right + node_has_disease;
    }
    return records_left + records_right;
}

int AVLT_Join_product_diseases(AVL_Tree* tree, Date* date1, Date* date2, char* disease){
    return AVLT_Join_product_diseases_r(tree->first_node,date1,date2, disease);
}


int AVLT_Join_product_ages_diseases_r(AVL_Tree_node* tree_node, Date* date1, Date* date2, char* disease, int age1, int age2){
    
    if(tree_node == NULL){
        return 0;
    }
    Date* patient_date = &(tree_node->patient->entry_date);
    char* patient_disease = tree_node->patient->disease_id;
    int records_right = 0;
    int records_left  = 0;
    // int num_of_patients = tree_node->num_of_patients;
    int compare1;
    int compare2;
    int node_has_disease = 0;

    if (strcmp(patient_disease, disease) == 0 && between(atoi(tree_node->patient->age), age1, age2) ){
        node_has_disease++;
    }

    Duplicate_Date_List *next_dublicate = tree_node->doublicate_date_list_first_node;
    while(next_dublicate != NULL){
        if (strcmp(next_dublicate->patient->disease_id, disease) == 0  && between(atoi(next_dublicate->patient->age), age1, age2) ){
            node_has_disease++;
        }
        next_dublicate = next_dublicate->next_node;
    }

    //date is conditional
    if(!(date1 == NULL || date2 == NULL)){
        compare1 = DATE_Compare(patient_date,date1);
        compare2 = DATE_Compare(patient_date,date2);
    }
    else{
        compare1 = 1;
        compare2 = -1;
    }
    //if patient is less or equal than the date1 then there is no point to search left node
    if (compare1 > 0)
        records_left = AVLT_Join_product_ages_diseases_r(tree_node->left_node,date1,date2, disease, age1, age2);

    //if patient is greater or equal than the date2 then there is no point to search right node
    if(compare2 < 0)
        records_right = AVLT_Join_product_ages_diseases_r(tree_node->right_node,date1,date2, disease, age1, age2);
    //if current node date is between the dates then return left+right +1
    // AND if they have the same disease as the query
    if( compare1 >= 0  &&  compare2 <= 0  &&  node_has_disease ){
        // printf("date->(%d) ",node_has_country); DATE_Print(patient_date);
        return records_left + records_right + node_has_disease;
    }
    return records_left + records_right;
}

int AVLT_Join_product_ages_diseases(AVL_Tree* tree, Date* date1, Date* date2, char* disease, int age1, int age2){
    return AVLT_Join_product_ages_diseases_r(tree->first_node,date1,date2, disease,  age1, age2);
}


AVL_Tree_node * AVLT_Destroy_r(AVL_Tree_node* tree_node){
    
    if(tree_node == NULL){
        return NULL;
    }

    AVLT_Destroy_r(tree_node->left_node);
    AVLT_Destroy_r(tree_node->right_node);
    destroy_douplicates(tree_node);
    free(tree_node);
}

void AVLT_Destroy(AVL_Tree* tree){
    AVLT_Destroy_r(tree->first_node);
    // printf("nodes-> (%d)\n",tree->nodes);
}

AVL_Tree_node * RR(AVL_Tree_node* tree_node){
    AVL_Tree_node* temp_right_node = tree_node->right_node;
    tree_node->right_node = tree_node->right_node->left_node;
    temp_right_node->left_node = tree_node;

    //Update depths
    tree_node->depth = 1 + max_depth(tree_node->left_node, tree_node->right_node);
    temp_right_node->depth = 1 + max_depth(temp_right_node->left_node, temp_right_node->right_node);

    return temp_right_node;
}

AVL_Tree_node * LL(AVL_Tree_node* tree_node){
    AVL_Tree_node* temp_left_node = tree_node->left_node;
    tree_node->left_node = tree_node->left_node->right_node;
    temp_left_node->right_node = tree_node;

    //Update depths
    tree_node->depth = 1 + max_depth(tree_node->left_node, tree_node->right_node);
    temp_left_node->depth = 1 + max_depth(temp_left_node->left_node, temp_left_node->right_node);

    return temp_left_node;
}

AVL_Tree_node * RL(AVL_Tree_node* tree_node){
    tree_node->right_node = LL(tree_node->right_node);
    return RR(tree_node);
}

AVL_Tree_node * LR(AVL_Tree_node* tree_node){
    tree_node->left_node = RR(tree_node->left_node);
    return LL(tree_node);
}

AVL_Tree_node * AVLT_Insert_r(AVL_Tree_node* tree_node, Patient_List_node* patient){

    AVL_Tree_node * node_to_insert, *next_tree_node;

    //If reached the end allocate new node and return in
    if(tree_node == NULL){
        return allocate_new_tree_node(patient);
    }
    
    int compare = AVLT_Date_compare(tree_node->patient, patient);
    if(compare > 0){
        tree_node->left_node = AVLT_Insert_r(tree_node->left_node, patient);
    }
    else if( compare < 0){
        tree_node->right_node = AVLT_Insert_r(tree_node->right_node, patient);
    }
    else{
        insert_doublicate(tree_node, patient);
        return tree_node;
    }

    //Update the depth
    tree_node->depth = 1 + max_depth(tree_node->left_node, tree_node->right_node);

    int rigth_depth = depth(tree_node->right_node);
    int left_depth  = depth(tree_node->left_node);
    int balance     = left_depth - rigth_depth;
    
    // Balance tree if balance > 1 <-1
    if(balance <= -2 && AVLT_Date_compare(patient, tree_node->right_node->patient) > 0 ){
        return RR(tree_node);
    }
    else if(balance <= -2 && AVLT_Date_compare(patient, tree_node->right_node->patient) < 0 ){
        return RL(tree_node);
    }
    else if(balance >= 2 && AVLT_Date_compare(patient, tree_node->left_node->patient) < 0 ){
        return LL(tree_node);
    }
    else if(balance >= 2 && AVLT_Date_compare(patient, tree_node->left_node->patient) > 0 ){
        return LR(tree_node);
    }



    return tree_node;
}

AVL_Tree_node * AVLT_Insert(AVL_Tree* tree, Patient_List_node* patient){

    tree->nodes++;
    //recursion insert
    tree->first_node = AVLT_Insert_r(tree->first_node, patient);

}