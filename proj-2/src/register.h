#ifndef SRC_REGISTER_H_
#define SRC_REGISTER_H_
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "./common.h"

#define IWANT 0
#define GOTRS 1
#define CLOSD 2
#define GAVUP 3

void register_op(Message msg, int type);

#endif  // SRC_REGISTER_H_
