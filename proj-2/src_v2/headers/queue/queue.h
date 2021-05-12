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

int queue_init(message_queue* q, int size);
int queue_push(message_queue* q, Message val);
int queue_pop(message_queue* q, Message* ret);
void queue_destroy(message_queue* q);
int queue_is_empty(message_queue* q);
void print_order(message_queue* q);
void print_arr(message_queue* q);
#endif  // HEADERS_QUEUE_QUEUE_H_
