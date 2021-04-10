#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

/**
 * Struct to store the information about a client thread (its id and priority of the task requested)
 */
typedef struct {
    int id;
    int priority;
} client_order;

void make_request();
void get_response();
void *client_thread_func(void * argument);

int parse_args(int argc, char* argv[]);
