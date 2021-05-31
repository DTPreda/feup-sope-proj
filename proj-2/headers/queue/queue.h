#ifndef HEADERS_QUEUE_QUEUE_H_
#define HEADERS_QUEUE_QUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include "common/common.h"

typedef struct {
    Message* arr;
    int* order;
    int size;
    int full;
} message_queue;

/**
 * Creates an empty queue.
 * @param q Struct used by the queue to work
 * @param size Size that the queue will have
 * @return 0 if successfull, 1 otherwise
 */
int queue_init(message_queue* q, int size);

/**
 * Adds an element to the queue, if it is not yet full
 * @param q Struct used by the queue to work
 * @param val Value to be added to the queue
 * @return 0 if successfull, 1 otherwise
 */ 
int queue_push(message_queue* q, Message val);

/**
 * Removes the first element of the queue
 * @param q Struct used by the queue to work
 * @param ret Value at the front of the queue, passed to this argument
 * @return 0 if successfull, 1 otherwise
 */ 
int queue_pop(message_queue* q, Message* ret);

/**
 * Destroy a no longer necessary queue
 * @param q Struct that works along with the queue, that is no longer needed
 */ 
void queue_destroy(message_queue* q);

/**
 * Check if the queue is empty or not
 * @param q Struct that contains the queue's data
 * @return 1 if empty, 0 otherwise
 */ 
int queue_is_empty(message_queue* q);

/**
 * Prints the contents of the queue, by order of insertion
 * @param q Struct that contains the queue's data
 */ 
void print_order(message_queue* q);

/**
 * Prints the array of messages, by order of insertion 
 * @param q Struct that contains the queue's data
 */ 
void print_arr(message_queue* q);

#endif  // HEADERS_QUEUE_QUEUE_H_
