#ifndef HEADERS_SERVER_H_
#define HEADERS_SERVER_H_
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "queue/common/common.h"
#include "lib/lib.h"
#include "queue/queue.h"

#define RECVD 0
#define TSKEX 1
#define TSKDN 2
#define TOOLATE 3
#define FAILD 4

#define POISON_PILL -10

/**
 * Calculates the remaining time left until timeout
 * @return The time left if no timeout achieved, 0 if out of time
 */ 
int get_remaining_time();

/**
 * Register an operation to the standard output
 * @param msg Message we wanto to output
 * @param type type of the message we want to send. One of (RECVD, TSKEX, TSKDN, 2LATE, FAILD)
 */ 
void register_operation(Message* msg, int type);

/**
 * Parse arguments introduce to ensure safety of the program
 * @param argc Number of arguments
 * @param argv Arguments given in the command line
 * @return 0 if using valid arguments, 1 otherwise
 */ 
int parse_arguments(int argc, char* argv[]);

/**
 * Initializes all the server structures including the queue, fifo and semaphore
 * @param q Struct used to work with the queue
 * @param bufsz Size of buffer used to hold values
 * @param fifo_name Public fifo to communicate with the client
 * @return 0 if initialization is successful, 1 otherwise
 */ 
int set_server_up(message_queue* q, int bufsz, char* fifo_name);

/**
 * Reads a request from the public FIFO specified
 * @param msg Variable used to hold the message received
 * @return 0 if successfull, 1 otherwise
 */ 
int get_request(Message* msg);

/**
 * Destroys all alocated resources to free up space
 */ 
void free_resources();

/**
 * Closes the public FIFO after a timeout, reading all the remaining 
 */ 
void close_fifo(pthread_t* tid, pthread_attr_t* attr);
void insert_item(Message* msg, message_queue* buffer);
void *attend_request(void* argument);
void send_result(Message* msg);
void *consumer_thread(void* argument);
int set_up_consumer_thread(pthread_t* pid, pthread_attr_t* attr);
void poison_pill();

#endif  // HEADERS_SERVER_H_
