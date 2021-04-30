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

/**
 * Gets the remaining time that program can run
 * @return time_t with the remaining time 
 */ 
time_t get_remaining_time();

/**
 * Parse arguments introduced to run the program
 * @return 1 if wrong arguments, 0 otherwise
 */ 
int parse_arguments(int argc, char* argv[]);

/**
 * Register an operation to standard output. 
 * @param Message message that we want to register 
 * @param type type of the message that we want to register (IWANT, GOTRS, CLOSD, GAVUP)
 */ 
void register_operation(Message msg, int type);

/**
 * Makes a request to the Servidor
 * @param Message request to be sent to Servidor
 * @return 1 if could not send the request, 0 otherwise
 */ 
int make_request(Message* msg);

/**
 * Registers the result of a received Message
 * @param Message message to be registered
 */ 
void register_result(Message msg);
int get_result(char* private_pipe, Message* msg);
int request_setup(char* private_pipe, Message* msg);
void *request(void* argument);

#endif  // PROJ_2_SRC_V2_HEADERS_CLIENT_H_
