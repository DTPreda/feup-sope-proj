#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>


typedef struct {
	int rid;	// request id
	pid_t pid;	// process id
	pthread_t tid;	// thread id
	int tskload;	// task load
	int tskres;	// task result
} message;

void make_request();
message get_response();
void *client_thread_func(void * argument);

int parse_args(int argc, char* argv[]);
