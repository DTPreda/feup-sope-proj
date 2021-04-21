#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include <pthread.h>


#define IWANT 0
#define GOTRS 1
#define CLOSD 2
#define GAVUP 3

void register_op(Message msg, int type);