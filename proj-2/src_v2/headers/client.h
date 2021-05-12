#ifndef HEADERS_CLIENT_H_
#define HEADERS_CLIENT_H_
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
#include "queue/common/common.h"

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

#endif  // HEADERS_CLIENT_H_
