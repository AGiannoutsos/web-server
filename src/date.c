#include "../includes/date.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void DATE_Insert(Date* date, char* string_date){
    char date_buf[MAX_DATE_SIZE];
    int i=0,j=0;


    while(string_date[j] != '-'){
        date_buf[i] = string_date[j];
        j++;
        i++;
    }
    date_buf[i] = '\0';
    date->day = atoi(date_buf);
    memset(date_buf, 0, MAX_DATE_SIZE*sizeof(char));

    j++;
    i=0;
    while(string_date[j] != '-'){
        date_buf[i] = string_date[j];
        j++;
        i++;
    }
    date_buf[i] = '\0';
    date->month = atoi(date_buf);
    memset(date_buf, 0, MAX_DATE_SIZE*sizeof(char));

    j++;
    i=0;
    while(string_date[j] != '\0'){
        date_buf[i] = string_date[j];
        j++;
        i++;
    }
    date_buf[i] = '\0';
    date->year = atoi(date_buf);
    memset(date_buf, 0, MAX_DATE_SIZE*sizeof(char));

}

void DATE_Print(Date* date){
    printf("d(%d) m(%d) y(%d)\n",date->day,date->month,date->year);
}

int DATE_Compare(Date* date1, Date* date2){
    //compare 2 dates if date1>date2 return > 0
    if(date1->year != date2->year)
        return date1->year - date2->year;
    else
        if(date1->month != date2->month)
            return date1->month - date2->month;
        else
            return date1->day - date2->day;
}

// campare vectors of strings
int DATE_Compare_string(const void * d1, const void * d2){
    Date date1;
    DATE_Insert(&date1, *((const char**)d1));

    Date date2;
    DATE_Insert(&date2, *((const char**)d2));

    return DATE_Compare(&date1, &date2);
}