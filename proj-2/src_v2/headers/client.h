#ifndef PROJ_2_SRC_V2_HEADERS_CLIENT_H_
#define PROJ_2_SRC_V2_HEADERS_CLIENT_H_
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "common/common.h"

#define IWANT 0
#define GOTRS 1
#define CLOSD 2
#define GAVUP 3

time_t get_remaining_time();
int parse_arguments(int argc, char* argv[]);
void register_operation(Message msg, int type);
int make_request(Message* msg);
void register_result(Message msg);
/**
 * Reads the result sent by the server for a previous successful request.
 * @param private_pipe - Name of the private FIFO through which the result will be read.
 * @param msg - Pointer to the Message struct where the result will be saved.
 * @return 0 if the result is correctly obtained; 1 otherwise.
*/
int get_result(char* private_pipe, Message* msg);
/**
 * Sets up required variables for a request.
 * @param private_pipe - Name of the private FIFO to be opened, through which the result of the request will be returned.
 * @param msg - Pointer to the Message to be sent to the server through its public FIFO.
 * @return 0 if all variables are correctly set up; 1 otherwise.
*/
int request_setup(char* private_pipe, Message* msg);
/**
 * Highest-level single request function, passed to every request-creation thread. 
 * @param argument - Argument passed to the function via pthread_create. Not utilized.
 * @return NULL on all cases.
*/
void *request(void* argument);
/**
 * Closes the global file descriptor referring to the server public FIFO.
*/
void close_public_fifo();

#endif  // PROJ_2_SRC_V2_HEADERS_CLIENT_H_
