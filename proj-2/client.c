#include "client.h" 

int inputTime;
time_t startTime;
char* public_pipe;
static int global_id = 0;
pthread_mutex_t access_public_pipe = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t control_id = PTHREAD_MUTEX_INITIALIZER;


/**
 * Make a request to Servidor
 * Probably need to pass the struct request throw argument and store it in public_pipe (putting it in public_pipe is making a request)
 */ 
void make_request(Message msg) {
    pthread_mutex_lock(&access_public_pipe);        
    
    // Writing the task Client want Servidor to perform
    int fd = open(public_pipe, O_WRONLY);  
    if (fd < 0) {
        // SERVIDOR FECHOU A FIFO, GUARDAR INFORMACAO
        fprintf(stdout, "Could not open public_pipe, make\n");
    } else {
        register_op(msg, IWANT);
        write(fd, &msg, sizeof(msg)); // waits...
    }

    close(fd);

    /* PROCESS MESSAGE, WRITE THAT SENT A REQUEST, ETC */
    pthread_mutex_unlock(&access_public_pipe);
}

time_t get_remaining_time(){
    time_t current_time = time(NULL);

    time_t ret = startTime + inputTime - current_time;
    if(ret < 0) return 0;
    return ret;
}

void read_message(int fd, Message* message){
    fd_set rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    time_t remaining_time = get_remaining_time();

    if (remaining_time == 0) {
        register_op(*message, GAVUP);
    } else {
        tv.tv_sec = remaining_time;
        tv.tv_usec = 0;

        int ret = select(fd + 1, &rfds, NULL, NULL, &tv);

        if (ret > 0) {
            // this mean one or more file descriptor has been written to
            read(fd, message, sizeof(*message));
            register_op(*message, GOTRS);
        } else if (ret == 0) {   
            // select timed out
            register_op(*message, GAVUP);
        } else {
            perror("select");
        }
    }
}

/**
 * Get response from a request. 
 * Probably need to return a struct request, read from the private pipe (idk how to read it from pipe, how does the output of the request come?)
 */ 
Message get_response() {
    char private_pipe[50];
    Message response;

    snprintf(private_pipe, 50, "/tmp/%d.%lu", getpid(), (unsigned long) pthread_self());

    // Writing the task Client want Servidor to perform
    int fd = open(private_pipe, O_RDONLY);
    if (fd < 0) {
        fprintf(stdout, "Could not open private_pipe\n");
    } else {
        read_message(fd, &response);
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

    make_request(order);

    Message response = get_response();   
    
    if(response.tskres == -1) {
        register_op(response, CLOSD);
        // terminate the program
    }
   
    if (remove(private_fifo) != 0){
        fprintf(stderr, "remove(private_fifo)\n");
    } 
    
    return(NULL);
}

int parse_args(int argc, char* argv[], int *inputTime) {
    if (argc != 4) return 1;
    if (strncmp(argv[1], "-t", 2) != 0) return 1;

    if (argv[2] <= 0) {
        return 1;
    } else {
        *inputTime = atoi(argv[2]);
    }

    public_pipe = argv[3];
    return 0;
}

int main(int argc, char* argv[]) {
    if (parse_args(argc, argv, &inputTime) != 0) {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }

    time(&startTime);
    
    // generate number between 1 and 9
    srand((unsigned) startTime);
    int upper = 9, lower = 1;
    int creationSleep = (rand() % (upper - lower + 1)) + lower;

    // time = 5, num = 1, max 50 threads (gave 10 + 1 to make sure it doesnt pass it)
    int maxNThreads = (inputTime * (10 + 1) / creationSleep);      
	
    pthread_t *ptid;
    ptid = (pthread_t *) malloc(maxNThreads * sizeof(pthread_t));
    
    int numThreads = 0;
    for (; ; numThreads++){
        usleep(creationSleep*pow(10, 5));       // this is 0.creationSleep seconds

        if (get_remaining_time() == 0) 
            break;
        
        pthread_create(&ptid[numThreads], NULL, client_thread_func, NULL);
    }

    for (int i = 0; i < numThreads; i++){
        pthread_join(ptid[i],NULL);
    }

    free(ptid);

    return 0;
}