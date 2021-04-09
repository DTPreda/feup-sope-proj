#include "client.h"
#include<fcntl.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

char* public_pipe;
static int global_id = 0;
pthread_mutex_t access_public_pipe;
sem_t access_private_pipe;


/**
 * Make a request to Servidor
 * Probably need to pass the struct request throw argument and store it in public_pipe 
 */ 
void make_request() {
    pthread_mutex_lock(&access_public_pipe);        // Just one thread accessing the public pipe to register requests
    int np;
    // Writing the task Client want Servidor to perform
    np = open(public_pipe, O_WRONLY);
    if (np < 0) {
        fprintf(stdout, "Could not open public_pipe\n");
    } else {
        write(np, "PEDIDO", 1+strlen("PEDIDO")); // waits...
    }
    /* PROCESS MESSAGE, WRITE THAT SENT A REQUEST, ETC */
    pthread_mutex_unlock(&access_public_pipe);
}

/**
 * Get response from a request. 
 * Probably need to return a struct request, read from the private pipe
 */ 
void get_response() {
    sem_wait(&access_private_pipe);
    char* private_pipe;
    char receivedMessage[1024];
    int np;
    // Writing the task Client want Servidor to perform
    np = open(public_pipe, O_RDONLY);
    if (np < 0) {
        fprintf(stdout, "Could not open public_pipe\n");
    } else {
        read(np, receivedMessage, 1024);
    }

    /* PROCESS MESSAGE, WRITE THAT RECEIVED THE RETURN OF THE REQUEST, ETC */
    sem_post(&access_private_pipe);
}

void *client_func(void * argument) {
    client_order order;
    // the client can access the semaphore and the Servidor to
    sem_init(&access_private_pipe, 0, 2);

    // generate number between 1 and 9
    int upper = 9, lower = 1;
    int num = (rand_r() % (upper - lower + 1)) + lower;
    order.id = global_id++;
    order.priority = num;

    char* private_pipe;
    snprintf(private_pipe, "/tmp/%d.%lu", gepid(), (unsigned long) pthread_self());
    if (mkfifo(private_pipe, 0666) < 0) return 1;     // public channel

    make_request();
}

int parse_args(int argc, char* argv[]) {
    if (argc != 4) return 1;
    if (strncmp(argv[1], "-t", 2) != 0) return 1;
    if (argv[2] <= 0) return 1;
    if (mkfifo(argv[3], 0666) < 0) return 1;     // public channel
    return 0;
}

int main(int argc, char* argv[]) {
    srand(time(0));

    if (parse_args(argc, argv) != 0) {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }

    public_pipe = argv[3];

    return 0;
}