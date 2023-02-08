#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <time.h>
#include <string.h>

#define BUF_LEN 256

int main(void)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

// Computing of current date and time
    char date[BUF_LEN] = {0};
    char buf[BUF_LEN] = {0};

    time_t rawtime = time(NULL);

    if (rawtime == -1) {

        puts("The time() function failed");
        return 1;
    }

    struct tm *ptm = localtime(&rawtime);

    if (ptm == NULL) {

        puts("The localtime() function failed");
        return 1;
    }

    memset(buf, 0, BUF_LEN);
    strftime(date, BUF_LEN, "%Y-%m-%d", ptm);

// Adding of current date into some log file path    
    char path[10] = {0};
    sprintf(path, "./var/log/folder/%s/log_name.log", date);  

// Reading from file counter (this file is required for adding data from log before rotation - coming of new day)
    int num;
    FILE *fptr;

    if ((fptr = fopen("/etc/zabbix/counter","r")) == NULL){
        printf("Error! opening file");

        // Program exits if the file pointer returns NULL.
        exit(1);
    }

// Adding of counter data into variable 'num'
   fscanf(fptr,"%d", &num);
   fclose(fptr);

// Opening of log file
    fp = fopen(path, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

// Creation of regular expressions
    regex_t regex;
    regex_t regex2;

    int reti;
    int reti2;

/*
for example, we will search lines with words 'removed' (routes), 'provider-1' (or some another word) and 'added' (routes), 'provider-1' (or some another word)
counter_R - count of lines with word 'removed' 
counter_R - count of lines with word 'added' 
*/

    int counter_R=0;
    int counter_A=0;

/*
difference = counter_R - counter_A.
'bad' and 'good' - possible states.
Bad - if difference > 0. It means that lines with word 'removed' more than lines with word 'added'.
*/

    int difference=0;
    int bad=1;
    int good=0;

// Compile 1 regular expression
    reti = regcomp(&regex, "removed.*provider-1", 0);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
    exit(1);
    }

// Compile 2 regular expression
    reti2 = regcomp(&regex2, "added.*provider-1", 1);
    if (reti2) {
        fprintf(stderr, "Could not compile regex\n");
    exit(1);
    }

// Main loop - read line by line, search regular expression and add count of matched lines into variables

    while ((read = getline(&line, &len, fp)) != -1) {

        reti = regexec(&regex, line, 0, NULL, 0);
        if (!reti) {
            counter_R++;
        }

        reti2 = regexec(&regex2, line, 0, NULL, 0);
        if (!reti2) {
            counter_A++;
        }
    }

// Computing difference between count of 'removed' and 'added' lines
    difference = counter_R - counter_A;

    if (difference + num > 0){
        printf("Result: %d,", bad);
    }
    else {
        printf("Result: %d,", good);
    }

// Current time
    time_t now = time(NULL);
    struct tm *tm_struct = localtime(&now);

    int hour = tm_struct->tm_hour;
    int min = tm_struct->tm_min;
    int sec = tm_struct->tm_sec;
    
// We will write the difference value into counter file before log rotation (at 23:59)
    if (hour == 23 & min == 59){
        if ((fptr = fopen("/etc/zabbix/counter","w")) == NULL){
            printf("Error! opening file");
            exit(1);
        }

        fprintf(fptr, "%d", difference);
        fclose(fptr);
    }

// Finally, it is necessary to free memory and close the log file
    regfree(&regex);
    regfree(&regex2);

    fclose(fp);
    if (line)
        free(line);
    exit(EXIT_SUCCESS);

    return 0;
}
