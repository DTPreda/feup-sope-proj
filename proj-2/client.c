#include "client.h"
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_CLIENT_RESULT -1

char* public_pipe;
static int global_id = 0;
pthread_mutex_t access_public_pipe;
pthread_mutex_t control_id;


/**
 * Make a request to Servidor
 * Probably need to pass the struct request throw argument and store it in public_pipe (putting it in public_pipe is making a request)
 */ 
void make_request(message msg) {
    pthread_mutex_lock(&access_public_pipe);        // Just one thread accessing the public pipe to register requests
    
    // Writing the task Client want Servidor to perform
    int fd = open(public_pipe, O_WRONLY);
    if (fd < 0) {
        fprintf(stdout, "Could not open public_pipe\n");
    } else {
        char message[40];
        snprintf(message, 40, "%d %d %d %lu %d", msg.rid, msg.tskload, msg.pid, msg.tid, msg.tskres);
        write(fd, message, 1 + strlen(message)); // waits...
    }

    close(fd);

    /* PROCESS MESSAGE, WRITE THAT SENT A REQUEST, ETC */
    pthread_mutex_unlock(&access_public_pipe);
}

/**
 * Get response from a request. 
 * Probably need to return a struct request, read from the private pipe (idk how to read it from pipe, how does the output of the request come?)
 */ 
message get_response() {
    char private_pipe[50];
    char receivedMessage[1024];
    message response;

    snprintf(private_pipe, 50, "/tmp/%d.%lu", getpid(), (unsigned long) pthread_self());


    // Writing the task Client want Servidor to perform
    int fd = open(private_pipe, O_RDONLY);
    if (fd < 0) {
        fprintf(stdout, "Could not open public_pipe\n");
    } else {
        read(fd, receivedMessage, 1024);
    }

    char * format;
    char msg[1024];

    format = strtok(receivedMessage, " ");
    snprintf(msg, 1024, "%s", format);
    response.rid = atoi(msg);	            // request id

    format = strtok(NULL, " ");
    snprintf(msg, 1024, "%s", format);
	response.tskload = atoi(msg);	        // task load

    format = strtok(NULL, " ");
    snprintf(msg, 1024, "%s", format);
	response.pid = atoi(msg);	            // process id

    format = strtok(NULL, " ");
    snprintf(msg, 1024, "%s", format);
    char *ptr;
	response.tid = strtol(msg, &ptr, 10);	// thread id

    format = strtok(NULL, " ");
    snprintf(msg, 1024, "%s", format);
	response.tskres = atoi(msg);	        // task result

    close(fd);


    /* PROCESS MESSAGE, WRITE THAT RECEIVED THE RETURN OF THE REQUEST, ETC */
    return response;
}

/**
 * Func to create client threads. It creates a private fifo to communicate with Servidor
 */
void *client_thread_func(void * argument) {
    // generate number between 1 and 9
    int upper = 9, lower = 1;
    unsigned int seed = time(0);
    srand(seed);
    int num = (rand_r(&seed) % (upper - lower + 1)) + lower;

    message order;
    order.pid = getpid();
    order.tid = pthread_self();
    order.tskload = num;
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

    make_request(order);

    message response = get_response();
    fprintf(stdout, "%d %d %d %lu %d", response.rid, response.tskload, response.pid, response.tid, response.tskres);
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
    ptid = (pthread_t *) malloc(2 * sizeof(pthread_t));
    public_pipe = argv[3];

    pthread_create(&ptid[0], NULL, client_thread_func, NULL);
    pthread_join(ptid[0],NULL);
    return 0;
}