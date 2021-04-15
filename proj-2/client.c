#include "client.h"
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_CLIENT_RESULT -1
#define NTHREADS 10

char* public_pipe;
static int global_id = 0;
pthread_mutex_t access_public_pipe;
pthread_mutex_t control_id;


/**
 * Make a request to Servidor
 * Probably need to pass the struct request throw argument and store it in public_pipe (putting it in public_pipe is making a request)
 */ 
void make_request(Message msg) {
    pthread_mutex_lock(&access_public_pipe);        // Just one thread accessing the public pipe to register requests
    
    // Writing the task Client want Servidor to perform
    int fd = open(public_pipe, O_WRONLY);  // WHY IS THIS BLOCKIIIING AND NOT OPENING PUBLIC PIPE
    if (fd < 0) {
        fprintf(stdout, "Could not open public_pipe\n");
    } else {
        write(fd, &msg, sizeof(msg)); // waits...
    }

    /*
    message response;
    read(fd, &response, sizeof(response));
    fprintf(stdout, "%d %d %d %lu %d\n", response.rid, response.tskload, response.pid, response.tid, response.tskres);
    */
    close(fd);

    /* PROCESS MESSAGE, WRITE THAT SENT A REQUEST, ETC */
    pthread_mutex_unlock(&access_public_pipe);
}

/**
 * Get response from a request. 
 * Probably need to return a struct request, read from the private pipe (idk how to read it from pipe, how does the output of the request come?)
 */ 
Message get_response() {
    char private_pipe[50];
    // char receivedMessage[1024];
    Message response;

    snprintf(private_pipe, 50, "/tmp/%d.%lu", getpid(), (unsigned long) pthread_self());

    // Writing the task Client want Servidor to perform
    int fd = open(private_pipe, O_RDONLY);
    if (fd < 0) {
        fprintf(stdout, "Could not open public_pipe\n");
    } else {
        read(fd, &response, sizeof(response));
    }

    close(fd);
    
    /* PROCESS Message, WRITE THAT RECEIVED THE RETURN OF THE REQUEST, ETC */
    return response;
}

/**
 * Func to create client threads. It creates a private fifo to communicate with Servidor
 */
void *client_thread_func(void * argument) {
    
    // generate number between 1 and 9
    int upper = 9, lower = 1;
    srand((unsigned) pthread_self());
    int num = (rand() % (upper - lower + 1)) + lower;

    Message order;
    order.tskload = num;
    order.pid = getpid();
    order.tid = pthread_self();
    order.tskres = DEFAULT_CLIENT_RESULT;

    pthread_mutex_lock(&control_id);
    order.rid = global_id;
    global_id++;
    pthread_mutex_unlock(&control_id);

    char private_fifo[50];
    snprintf(private_fifo, 50, "/tmp/%d.%lu", getpid(), (unsigned long) pthread_self());
    if (mkfifo(private_fifo, 0666) < 0){
        fprintf(stderr, "mkfifo()");
    }     // private channel

    fprintf(stdout, "%d %d %d %lu %d\n", order.rid, order.tskload, order.pid, order.tid, order.tskres);

    make_request(order);

    Message response = get_response();   
    
    // DEBUG
    fprintf(stdout, "Message:%d %d %d %lu %d\n", response.rid, response.tskload, response.pid, response.tid, response.tskres);
    
    if (remove(private_fifo) != 0){
        fprintf(stderr, "remove(private_fifo)\n");
    } 
    
    return(NULL);
}

int parse_args(int argc, char* argv[]) {
    if (argc != 4) return 1;
    if (strncmp(argv[1], "-t", 2) != 0) return 1;
    if (argv[2] <= 0) return 1;
    if (mkfifo(argv[3], 0666) < 0) return 1;     // public channel
    return 0;
}

int main(int argc, char* argv[]) {
    parse_args(argc, argv);
    /*if (parse_args(argc, argv) != 0) {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }*/

	pthread_t *ptid;
    ptid = (pthread_t *) malloc(NTHREADS * sizeof(pthread_t));
    public_pipe = argv[3];
    
    for (int i = 0; i < NTHREADS; i++){
        pthread_create(&ptid[i], NULL, client_thread_func, NULL);
    }
    for (int i = 0; i < NTHREADS; i++){
        pthread_join(ptid[i],NULL);
    }
    return 0;
}