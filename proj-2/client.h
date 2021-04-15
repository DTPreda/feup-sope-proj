#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "common.h"

void make_request(Message msg);
Message get_response();
void *client_thread_func(void * argument);

int parse_args(int argc, char* argv[], int *inputTime);
