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
#include "common.h"
#include "lib.h"
#include "queue.h"

#define RECVD 0
#define TSKEX 1
#define TSKDN 2
#define TOOLATE 3
#define FAILD 4

#define POISON_PILL -10

int get_remaining_time();
void register_operation(Message* msg, int type);
int parse_arguments(int argc, char* argv[]);
int set_server_up(message_queue* q, int bufsz, char* fifo_name);
int get_request(Message* msg);
void free_resources();
void close_fifo(pthread_t* tid, pthread_attr_t* attr);
void insert_item(Message* msg, message_queue* buffer);
void *attend_request(void* argument);
void send_result(Message* msg);
void *consumer_thread(void* argument);
int set_up_consumer_thread(pthread_t* pid, pthread_attr_t* attr);
void poison_pill();

#endif  // HEADERS_SERVER_H_
