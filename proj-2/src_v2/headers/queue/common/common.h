#ifndef HEADERS_QUEUE_COMMON_COMMON_H_
#define HEADERS_QUEUE_COMMON_COMMON_H_ 1
typedef struct {
int rid;        // request id
pid_t pid;      // process id
pthread_t tid;  // thread id
int tskload;    // task load
int tskres;     // task result
} Message;
#endif  // HEADERS_QUEUE_COMMON_COMMON_H_
