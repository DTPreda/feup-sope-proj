#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "common.h"

/*
typedef struct {
	int rid;	// request id
	pid_t pid;	// process id
	pthread_t tid;	// thread id
	int priority;	// priority of the task
	int res;	// result of the task
} message; */

void make_request(Message msg);
Message get_response();
void *client_thread_func(void * argument);

int parse_args(int argc, char* argv[]);
