#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "headers/common/common.h"
#include "queue.h"
#include "headers/lib.h"

#define RECVD 0
#define TSKEX 1
#define TSKDN 2
#define TOOLATE 3
#define FAILD 4

#define POISON_PILL -10

int get_remaining_time();
void register_operation(Message* msg, int type);
int parse_arguments(int argc, char* argv[]);