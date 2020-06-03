#ifndef DATE_H
#define DATE_H

#define MAX_DATE_SIZE 5

typedef struct Date{
    int day;
    int month;
    int year;
} Date;


void DATE_Insert(Date* date, char* string_date);

void DATE_Print(Date* date);
int DATE_Compare(Date* date1, Date* date2);
int DATE_Compare_string(const void* d1, const void* d2);

#endif