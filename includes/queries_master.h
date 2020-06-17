#ifndef QUERIES_MASTER_H
#define QUERIES_MASTER_H

#include "../includes/message_handlers.h"
#include "../includes/util.h"

int list_countries(Worker* worker, int num_of_workers);

int disease_frequency(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, int buffer_size);

int topk_age_ranges(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, int buffer_size);

int search_patient_record(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, int buffer_size);

int num_patients_admissions_discharges(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, int buffer_size);

// server version 
// same but also they copy results to results messae
int disease_frequency_server(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, Message_vector* results_printed, int buffer_size);

int topk_age_ranges_server(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, Message_vector* results_printed, int buffer_size);

int search_patient_record_server(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, Message_vector* results_printed, int buffer_size);

int num_patients_admissions_discharges_server(int* write_fds, int* read_fds, int num_of_workers,  Message_vector* message, Message_vector* results_printed, int buffer_size);

#endif