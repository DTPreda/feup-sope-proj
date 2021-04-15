#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include "register.h"
#include "common.h"

#define DEFAULT_CLIENT_RESULT -1
#define NTHREADS 10

void make_request(Message msg);
Message get_response();
void *client_thread_func(void * argument);
int parse_args(int argc, char* argv[]);
