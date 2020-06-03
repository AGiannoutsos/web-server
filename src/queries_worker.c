#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/message_handlers.h"
#include "../includes/util.h"
#include "../includes/queries_worker.h"
#include "../includes/hashtable.h"
#include "../includes/patient_list.h"
#include "../includes/avl_tree.h"
#include "../includes/ht_avlt.h"

int disease_frequency(Message_vector* results, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table){
    if(num_of_args !=4 && num_of_args !=5){
        // printf("error\n");
        // -1 for error
        results->num_of_args = 1;
        results->args = malloc(1*sizeof(char*));
        char buffer_cases[32];
        sprintf(buffer_cases, "%d", -1);
        results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
        strcpy(results->args[0], buffer_cases);
        return -1;
    }
    AVL_Tree* tree;
    int cases=0;
    Date d1,d2;
    Hash_Table_bucket* null_bucket;
    Message_Delete(results);

    if(num_of_args == 4){
        //get the disease avl tree
        tree = HT_AVLT_Search(disease_hash_table, args[1], &null_bucket);
        if(tree == NULL){
            // printf("error\n");
            // 0 results
            results->num_of_args = 1;
            results->args = malloc(1*sizeof(char*));
            char buffer_cases[32];
            sprintf(buffer_cases, "%d", 0);
            results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
            strcpy(results->args[0], buffer_cases);
            return 1;
        }

        DATE_Insert(&d1, args[2]);
        DATE_Insert(&d2, args[3]);

        cases = AVLT_Records_between(tree, &d1, &d2);
        // printf("%s %d\n", args[1], cases);
        // sleep(10);

    }
    else if(num_of_args == 5){
        //get the disease avl tree
        tree = HT_AVLT_Search(disease_hash_table, args[1], &null_bucket);
        if(tree == NULL){
            // printf("error\n");
            // 0 results
            results->num_of_args = 1;
            results->args = malloc(1*sizeof(char*));
            char buffer_cases[32];
            sprintf(buffer_cases, "%d", 0);
            results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
            strcpy(results->args[0], buffer_cases);
            return 1;
        }

        DATE_Insert(&d1, args[2]);
        DATE_Insert(&d2, args[3]);

        cases = AVLT_Join_product_countries(tree, &d1, &d2, args[4]);
        // printf("%s %d\n", args[1], cases);

    }
    // copy results
    results->num_of_args = 1;
    results->args = malloc(1*sizeof(char*));
    char buffer_cases[32];
    sprintf(buffer_cases, "%d", cases);
    results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
    strcpy(results->args[0], buffer_cases);
    return 1;
}


int write_disease_frequency(int write_fd, int buffer_size, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table){

    Message_vector results;
    Message_Init(&results);
    int return_status = disease_frequency(&results, num_of_args, args, patient_list, disease_hash_table);

    Message_Write(write_fd, &results, buffer_size);
    Message_Write_End_Com(write_fd, buffer_size);
    Message_Delete(&results);

    return return_status;
}

struct ages_groups{
        float age;
        char* group;
};
int group_comparator(const void *g1, const void *g2){
    if ( (((struct ages_groups *)g1)->age) == (((struct ages_groups *)g2)->age) )
        return 0;

    return   (((struct ages_groups *)g1)->age) > (((struct ages_groups *)g2)->age)   ?  -1 : 1;
} 


int topk_age_ranges(Message_vector* results, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table){
    if(num_of_args !=6){
        // printf("error\n");
        // -1 for error
        results->num_of_args = 1;
        results->args = malloc(1*sizeof(char*));
        char buffer_cases[32];
        sprintf(buffer_cases, "%d", -1);
        results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
        strcpy(results->args[0], buffer_cases);
        return -1;
    }
    AVL_Tree* tree;
    // int cases=0;
    Date d1,d2;
    Hash_Table_bucket* null_bucket;
    char* disease;
    Message_Delete(results);

    //get the country avl tree
    tree = HT_AVLT_Search(country_hash_table, args[2], &null_bucket);
    if(tree == NULL){
        // printf("error\n");
        // 0 results
        results->num_of_args = 1;
        results->args = malloc(1*sizeof(char*));
        char buffer_cases[32];
        sprintf(buffer_cases, "%d", 0);
        results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
        strcpy(results->args[0], buffer_cases);
        return 1;
    }
    disease = args[3];
    int k = atoi(args[1]);

    // int* cases = malloc(4*sizeof(int));
    float total_cases = 0;
   
    struct ages_groups* cases;
    cases = malloc(4*sizeof(struct ages_groups));
    memset(cases, 0, 4*sizeof(struct ages_groups));

    cases[0].group = "0-20";
    cases[1].group = "21-40";
    cases[2].group = "41-60";
    cases[3].group = "60+";

    // get the cases in that tree with the given age
    DATE_Insert(&d1, args[4]);
    DATE_Insert(&d2, args[5]);
    // get num of cases from every age group
    cases[0].age = (float)AVLT_Join_product_ages_diseases(tree, &d1, &d2, disease, 0, 20);
    cases[1].age = (float)AVLT_Join_product_ages_diseases(tree, &d1, &d2, disease, 21, 40);
    cases[2].age = (float)AVLT_Join_product_ages_diseases(tree, &d1, &d2, disease, 41, 60);
    cases[3].age = (float)AVLT_Join_product_ages_diseases(tree, &d1, &d2, disease, 61, 120);

    total_cases = cases[0].age + cases[1].age + cases[2].age + cases[3].age;
    // printf("tolal -> %f\n",total_cases);

    // If total cases 0 then exit with no error
    if(total_cases == 0 || k == 0){
        // printf("error\n");
        // 0 results
        results->num_of_args = 1;
        results->args = malloc(1*sizeof(char*));
        char buffer_cases[32];
        sprintf(buffer_cases, "%d", 0);
        results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
        strcpy(results->args[0], buffer_cases);
        return 1;
    }

    // sort results
    qsort(cases, 4, sizeof(struct ages_groups), group_comparator);

    // get mean
    for (int i = 0; i < 4; i++){
        cases[i].age = cases[i].age / total_cases * 100;
    }


    // copy sort to results
    results->num_of_args = k*2;
    results->args = malloc(k*2*sizeof(char*));
    char buffer_cases[32];
    memset(buffer_cases, 0, 32);
    for (int i = 0; i < k*2; i+=2){
        results->args[i] = malloc((strlen(cases[i/2].group)+1)*sizeof(char));
        strcpy(results->args[i], cases[i/2].group);

        sprintf(buffer_cases, "%.0f", cases[i/2].age);
        results->args[i+1] = malloc((strlen(buffer_cases)+1)*sizeof(char));
        strcpy(results->args[i+1], buffer_cases);
        memset(buffer_cases, 0, 32);
    }

    free(cases);
    return 1;

}


int write_topk_age_ranges(int write_fd, int buffer_size, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table){

    Message_vector results;
    Message_Init(&results);
    int return_status = topk_age_ranges(&results, num_of_args, args, patient_list, disease_hash_table, country_hash_table);

    Message_Write(write_fd, &results, buffer_size);
    Message_Write_End_Com(write_fd, buffer_size);
    Message_Delete(&results);

    return return_status;
}


int search_patient_record(Message_vector* results, int num_of_args, char** args, Patient_List* patient_list){
    if (num_of_args != 2){
        // printf("error\n");
        // -1 for error
        results->num_of_args = 1;
        results->args = malloc(1*sizeof(char*));
        char buffer_cases[32];
        sprintf(buffer_cases, "%d", -1);
        results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
        strcpy(results->args[0], buffer_cases);
        return -1;
    }

    Message_Delete(results);

    // scan patient list
    Patient_List_node* curr_patinet = patient_list->first_node;
    int found = 0;

    while(curr_patinet != NULL){

        // if found the id then return node and break
        if(strcmp(curr_patinet->record_id, args[1]) == 0){
            found = 1;
            break;
        }

        curr_patinet = curr_patinet->next_node;
    }

    // if not found return 0
    if (found == 0){
        // 0 results
        results->num_of_args = 1;
        results->args = malloc(1*sizeof(char*));
        char buffer_cases[32];
        sprintf(buffer_cases, "%d", 0);
        results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
        strcpy(results->args[0], buffer_cases);
        return 1;
    }

    // copy patient to results if found
    results->num_of_args = 7;
    results->args = malloc(7*sizeof(char*));
    memset(results->args, 0, 7*sizeof(char*));

    // id
    results->args[0] = malloc((strlen(curr_patinet->record_id)+1)*sizeof(char));
    strcpy(results->args[0], curr_patinet->record_id);

    // first name
    results->args[1] = malloc((strlen(curr_patinet->fisrt_name)+1)*sizeof(char));
    strcpy(results->args[1], curr_patinet->fisrt_name);

    // last name
    results->args[2] = malloc((strlen(curr_patinet->last_name)+1)*sizeof(char));
    strcpy(results->args[2], curr_patinet->last_name);

    // disease
    results->args[3] = malloc((strlen(curr_patinet->disease_id)+1)*sizeof(char));
    strcpy(results->args[3], curr_patinet->disease_id);

    // age
    results->args[4] = malloc((strlen(curr_patinet->age)+1)*sizeof(char));
    strcpy(results->args[4], curr_patinet->age);

    // entry date
    char date_string[64] = {0};
    sprintf(date_string, "%d-%d-%d", curr_patinet->entry_date.day, curr_patinet->entry_date.month, curr_patinet->entry_date.year);
    results->args[5] = malloc((strlen(date_string)+1)*sizeof(char));
    strcpy(results->args[5], date_string);

    // if has exit date print it or print --
    if(curr_patinet->valid_exit_date == 1){
        memset(date_string, 0, 64);
        sprintf(date_string, "%d-%d-%d", curr_patinet->exit_date.day, curr_patinet->exit_date.month, curr_patinet->exit_date.year);
        results->args[6] = malloc((strlen(date_string)+1)*sizeof(char));
        strcpy(results->args[6], date_string);
    }
    else{
        memset(date_string, 0, 64);
        sprintf(date_string, "--");
        results->args[6] = malloc((strlen(date_string)+1)*sizeof(char));
        strcpy(results->args[6], date_string);
    }

    return 1;
}

int write_search_patient_record(int write_fd, int buffer_size, int num_of_args, char** args, Patient_List* patient_list){

    Message_vector results;
    Message_Init(&results);
    int return_status = search_patient_record(&results, num_of_args, args, patient_list);

    Message_Write(write_fd, &results, buffer_size);
    Message_Write_End_Com(write_fd, buffer_size);
    Message_Delete(&results);

    return return_status;
}

int num_patient_admissions(Message_vector* results, Worker* me_worker, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table){
    if(num_of_args !=4 && num_of_args !=5){
        // printf("error\n");
        // -1 for error
        results->num_of_args = 1;
        results->args = malloc(1*sizeof(char*));
        char buffer_cases[32];
        sprintf(buffer_cases, "%d", -1);
        results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
        strcpy(results->args[0], buffer_cases);
        return -1;
    }
    AVL_Tree* tree;
    int cases=0;
    Date d1,d2;
    Hash_Table_bucket* null_bucket;
    Message_Delete(results);


    if(num_of_args == 4){
        // print all the countries
        // get the number of them and malloc for the results
        // if country has 0 cases then print it and put 0
        // if no disease then return 0
        tree = HT_AVLT_Search(disease_hash_table, args[1], &null_bucket);
        if(tree == NULL){
            // printf("error\n");
            // 0 results
            results->num_of_args = 1;
            results->args = malloc(1*sizeof(char*));
            char buffer_cases[32] = {0};
            sprintf(buffer_cases, "%d", 0);
            results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
            strcpy(results->args[0], buffer_cases);
            return 1;
        }

        int num_of_countries = me_worker->num_of_directories;
        results->num_of_args = 2*num_of_countries;
        results->args = malloc(2*num_of_countries*sizeof(char*));
        int arg = 0;
        char buffer_cases[32] = {0};
        DATE_Insert(&d1, args[2]);
        DATE_Insert(&d2, args[3]);


        char *country = HT_AVLT_Get_key(country_hash_table); 
        while(country != NULL){   
            // get the cases in that tree with the given country
            cases = AVLT_Join_product_countries(tree, &d1, &d2, country);
            // copy cases to results
            results->args[arg] = malloc((strlen(country)+1)*sizeof(char));
            strcpy(results->args[arg], country);
            // num of cases
            sprintf(buffer_cases, "%d", cases);
            results->args[arg+1] = malloc((strlen(buffer_cases)+1)*sizeof(char));
            strcpy(results->args[arg+1], buffer_cases);
            memset(buffer_cases, 0, 32);
            // printf("%s -> %d cases in %s\n\n", args[2], cases, country);
            country = HT_AVLT_Get_key(country_hash_table); 
            arg+=2;
        }
        return 1;
    }
    else if(num_of_args == 5){
        //get the disease avl tree
        tree = HT_AVLT_Search(disease_hash_table, args[1], &null_bucket);
        if(tree == NULL){
            // printf("error\n");
            // 0 results
            results->num_of_args = 1;
            results->args = malloc(1*sizeof(char*));
            char buffer_cases[32] = {0};
            sprintf(buffer_cases, "%d", 0);
            results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
            strcpy(results->args[0], buffer_cases);
            return 1;
        }

        // if worker has no that country worker print 0
        AVL_Tree* country_tree = HT_AVLT_Search(country_hash_table, args[4], &null_bucket);
        if(country_tree == NULL){
            // printf("error\n");
            // 0 results
            results->num_of_args = 1;
            results->args = malloc(1*sizeof(char*));
            char buffer_cases[32];
            sprintf(buffer_cases, "%d", 0);
            results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
            strcpy(results->args[0], buffer_cases);
            return 1;
        }

        DATE_Insert(&d1, args[2]);
        DATE_Insert(&d2, args[3]);

        cases = AVLT_Join_product_countries(tree, &d1, &d2, args[4]);
        // printf("%s %d\n", args[1], cases);

        // copy results
        results->num_of_args = 2;
        results->args = malloc(2*sizeof(char*));
        char buffer_cases[64] = {0};

        results->args[0] = malloc((strlen(args[4])+1)*sizeof(char));
        strcpy(results->args[0], args[4]);

        sprintf(buffer_cases, "%d", cases);
        results->args[1] = malloc((strlen(buffer_cases)+1)*sizeof(char));
        strcpy(results->args[1], buffer_cases);


        return 1;

    }
}


int write_num_patient_admissions(int write_fd, int buffer_size, Worker* me_worker, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table){

    Message_vector results;
    Message_Init(&results);
    int return_status = num_patient_admissions(&results, me_worker, num_of_args, args, patient_list, disease_hash_table, country_hash_table);

    Message_Write(write_fd, &results, buffer_size);
    // Message_Print(&results);
    Message_Write_End_Com(write_fd, buffer_size);
    Message_Delete(&results);

    return return_status;
}


int num_patient_discharges(Message_vector* results, Worker* me_worker, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table){
    if(num_of_args !=4 && num_of_args !=5){
        // printf("error\n");
        // -1 for error
        results->num_of_args = 1;
        results->args = malloc(1*sizeof(char*));
        char buffer_cases[32];
        sprintf(buffer_cases, "%d", -1);
        results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
        strcpy(results->args[0], buffer_cases);
        return -1;
    }
    AVL_Tree* tree;
    int cases=0;
    Date d1,d2;
    Hash_Table_bucket* null_bucket;
    Message_Delete(results);


    if(num_of_args == 4){
        // print all the countries
        // get the number of them and malloc for the results
        // if country has 0 cases then print it and put 0
        
        tree = HT_AVLT_Search(disease_hash_table, args[1], &null_bucket);
        if(tree == NULL){
            // printf("error\n");
            // 0 results
            results->num_of_args = 1;
            results->args = malloc(1*sizeof(char*));
            char buffer_cases[32] = {0};
            sprintf(buffer_cases, "%d", 0);
            results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
            strcpy(results->args[0], buffer_cases);
            return 1;
        }

        int num_of_countries = me_worker->num_of_directories;
        results->num_of_args = 2*num_of_countries;
        results->args = malloc(2*num_of_countries*sizeof(char*));
        int arg = 0;
        char buffer_cases[32] = {0};
        DATE_Insert(&d1, args[2]);
        DATE_Insert(&d2, args[3]);


        Patient_List_node* curr_patinet;
        int compare1 = 0;
        int compare2 = 0;
        char *country = HT_AVLT_Get_key(country_hash_table); 
        while(country != NULL){   
            // get the cases in that tree with the given country
            // cases = AVLT_Join_product_countries_exit_date(tree, &d1, &d2, country);
            cases = 0;
            curr_patinet = patient_list->first_node;
            while(curr_patinet != NULL){
                if( curr_patinet->valid_exit_date == 1 && strcmp(curr_patinet->country, country) == 0 && strcmp(curr_patinet->disease_id, args[1]) == 0 ){
                    // find if record is in between
                    compare1 = DATE_Compare(&(curr_patinet->exit_date), &d1);
                    compare2 = DATE_Compare(&(curr_patinet->exit_date), &d2);
                    // if found the id then return node and break
                    if( compare1 >= 0  &&  compare2 <= 0 ){
                        cases++;
                    }
                }
                curr_patinet = curr_patinet->next_node;
            }

            // copy cases to results
            results->args[arg] = malloc((strlen(country)+1)*sizeof(char));
            strcpy(results->args[arg], country);
            // num of cases
            sprintf(buffer_cases, "%d", cases);
            results->args[arg+1] = malloc((strlen(buffer_cases)+1)*sizeof(char));
            strcpy(results->args[arg+1], buffer_cases);
            memset(buffer_cases, 0, 32);
            // printf("%s -> %d cases in %s\n\n", args[2], cases, country);
            country = HT_AVLT_Get_key(country_hash_table); 
            arg+=2;
        }

        return 1;
    }
    else if(num_of_args == 5){
        //get the disease avl tree
        tree = HT_AVLT_Search(disease_hash_table, args[1], &null_bucket);
        if(tree == NULL){
            // printf("error\n");
            // 0 results
            results->num_of_args = 1;
            results->args = malloc(1*sizeof(char*));
            char buffer_cases[32] = {0};
            sprintf(buffer_cases, "%d", 0);
            results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
            strcpy(results->args[0], buffer_cases);
            return 1;
        }

        // if worker has no that country worker print 0
        AVL_Tree* country_tree = HT_AVLT_Search(country_hash_table, args[4], &null_bucket);
        if(country_tree == NULL){
            // printf("error\n");
            // 0 results
            results->num_of_args = 1;
            results->args = malloc(1*sizeof(char*));
            char buffer_cases[32];
            sprintf(buffer_cases, "%d", 0);
            results->args[0] = malloc((strlen(buffer_cases)+1)*sizeof(char));
            strcpy(results->args[0], buffer_cases);
            return 1;
        }

        DATE_Insert(&d1, args[2]);
        DATE_Insert(&d2, args[3]);

        // cases = AVLT_Join_product_countries_exit_date(tree, &d1, &d2, args[4]);
        Patient_List_node* curr_patinet;
        int compare1 = 0;
        int compare2 = 0;
        cases = 0;
        curr_patinet = patient_list->first_node;
        while(curr_patinet != NULL){
            if( curr_patinet->valid_exit_date == 1 && strcmp(curr_patinet->country, args[4]) == 0 && strcmp(curr_patinet->disease_id, args[1]) == 0 ){
                // find if record is in between
                compare1 = DATE_Compare(&(curr_patinet->exit_date), &d1);
                compare2 = DATE_Compare(&(curr_patinet->exit_date), &d2);
                // if found the id then return node and break
                if( compare1 >= 0  &&  compare2 <= 0 ){
                    cases++;
                }
            }
            curr_patinet = curr_patinet->next_node;
        }


        // printf("%s %d\n", args[1], cases);

        // copy results
        results->num_of_args = 2;
        results->args = malloc(2*sizeof(char*));
        char buffer_cases[64] = {0};

        results->args[0] = malloc((strlen(args[4])+1)*sizeof(char));
        strcpy(results->args[0], args[4]);

        sprintf(buffer_cases, "%d", cases);
        results->args[1] = malloc((strlen(buffer_cases)+1)*sizeof(char));
        strcpy(results->args[1], buffer_cases);


        return 1;

    }
}


int write_num_patient_discharges(int write_fd, int buffer_size, Worker* me_worker, int num_of_args, char** args, Patient_List* patient_list, Hash_Table* disease_hash_table, Hash_Table* country_hash_table){

    Message_vector results;
    Message_Init(&results);
    int return_status = num_patient_discharges(&results, me_worker, num_of_args, args, patient_list, disease_hash_table, country_hash_table);

    Message_Write(write_fd, &results, buffer_size);
    Message_Write_End_Com(write_fd, buffer_size);
    Message_Delete(&results);

    return return_status;
}