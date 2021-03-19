#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "./xmod.h"
#include "./log.h"

struct timespec start_time, end_time;
long int time_start, time_end;


int log_start() {
    if (getpid() != FIRST_PROCESS_PID) {
        if(getenv(START_TIME) != NULL){
            time_start = atol(getenv(START_TIME));
        } else {
            return 1;
        }
    } else {
        clock_gettime(CLOCK_REALTIME, &start_time);
        time_start = start_time.tv_sec * 1000 + start_time.tv_nsec/(pow(10, 6));

        char st_time[50];
        snprintf(st_time, sizeof(st_time) , "%ld", time_start);

        if (setenv(START_TIME, st_time, 1) == -1) {
            fprintf(stderr, "Error setting environment variable\n");
            return 1;
        }

        char* log_file_name = getenv(LOG_FILENAME);

        if (log_file_name != NULL) {
            FILE* log_file = fopen(log_file_name, "w");
            fclose(log_file);
        }
    }

    return 0;
}


void write_to_log(unsigned int event, char* info) {
    char* log_file_name = getenv(LOG_FILENAME);
    if (log_file_name != NULL) {
        FILE* log_file;

        log_file = fopen(log_file_name, "a");

        char str[FILENAME_MAX];
        switch (event) {
        case PROC_CREATE:
            snprintf(str, sizeof(str), "%ld ; %d ; PROC_CREATE ;%s\n", get_running_time() - time_start, getpid(), info);
            break;
        case PROC_EXIT:
            snprintf(str, sizeof(str), "%ld ; %d ; PROC_EXIT ; %s\n", get_running_time() - time_start, getpid(), info);
            break;
        case SIGNAL_RECV:
            snprintf(str, sizeof(str), "%ld ; %d ; SIGNAL_RECV ; %s\n", get_running_time() - time_start, getpid(), info);
            break;
        case SIGNAL_SENT:
            snprintf(str, sizeof(str), "%ld ; %d ; SIGNAL_SENT ; %s\n", get_running_time() - time_start, getpid(), info);
            break;
        case FILE_MODF:
            snprintf(str, sizeof(str), "%ld ; %d ; FILE_MODF ; %s\n", get_running_time() - time_start, getpid(), info);
        default:
            break;
        }

        fputs(str, log_file);
        fclose(log_file);
    }
}

long int get_running_time() {
    clock_gettime(CLOCK_REALTIME, &end_time);
    long int delta_ms = end_time.tv_sec * 1000 + end_time.tv_nsec/(pow(10, 6));   // time in ms
    return delta_ms;
}
