#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "common.h"

#define IWANT 0
#define GOTRS 1
#define CLOSD 2
#define GAVUP 3

time_t get_remaining_time();
int parse_arguments(int argc, char* argv[]);
void register_operation(Message msg, int type);
int make_request(Message* msg);
void register_result(Message msg);
int get_result(char* private_pipe, Message* msg);
int request_setup(char* private_pipe, Message* msg);
void *request(void* argument);
