#ifndef PROJ_2_SRC_V2_COMMON_H_
#define PROJ_2_SRC_V2_COMMON_H_ 1
typedef struct {
int rid;        // request id
pid_t pid;      // process id
pthread_t tid;  // thread id
int tskload;    // task load
int tskres;     // task result
} Message;
#endif  // PROJ_2_SRC_V2_COMMON_H_
