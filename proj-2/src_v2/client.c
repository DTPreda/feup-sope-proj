#include "./client.h"

int inputTime;
time_t startTime;
char* public_pipe;
int public_pipe_fd;
int global_rid = 0;
volatile int is_closd = 0;
pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;

time_t get_remaining_time() {
    time_t current_time = time(NULL);

    time_t ret = startTime + inputTime - current_time;
    if (ret < 0) return 0;
    return ret;
}

int parse_arguments(int argc, char* argv[]) {
    if (argc != 4) return 1;
    if (strncmp(argv[1], "-t", 2) != 0) return 1;
    if (atoi(argv[2]) < 0) return 1;

    inputTime = atoi(argv[2]);
    public_pipe = argv[3];

    return 0;
}

void register_operation(Message msg, int type) {
    time_t t = time(NULL);
    if (t == -1) return;

    pthread_mutex_lock(&output_mutex);
    fprintf(stdout, "%li ; %i ; %i ; %i ; %lu ; %i ; ", t, msg.rid, msg.tskload, msg.pid , (unsigned long) msg.tid, msg.tskres);
    switch (type) {
        case(IWANT):
            fprintf(stdout, "IWANT");
            break;
        case(GOTRS):
            fprintf(stdout, "GOTRS");
            break;
        case(CLOSD):
            fprintf(stdout, "CLOSD");
            break;
        case(GAVUP):
            fprintf(stdout, "GAVUP");
            break;
        default:
            break;
    }
    fprintf(stdout, "\n");
    pthread_mutex_unlock(&output_mutex);
}

int make_request(Message* msg) {
    int sl;

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(public_pipe_fd, &wfds);

    struct timeval tv;
    tv.tv_sec = get_remaining_time();
    tv.tv_usec = 0;

    if ((sl = select(public_pipe_fd + 1, NULL, &wfds, NULL, &tv)) == -1) {
        perror("select");
        return 1;
    } else if (sl > 0) {
        if (write(public_pipe_fd, msg, sizeof(*msg)) == -1) {
            // perror("write");
            return 1;
        }
        register_operation(*msg, IWANT);
    } else if (sl == 0) {  // timeout
        // register_operation(*msg, GAVUP);
        return 1;
    }

    return 0;
}

void register_result(Message msg) {
    if (msg.tskres != -1) {
       register_operation(msg, GOTRS);
    } else {
        register_operation(msg, CLOSD);

        // pthread_mutex_lock(&closd_mutex);
        is_closd = 1;
        // pthread_mutex_unlock(&closd_mutex);
    }
}

int get_result(char* private_pipe, Message* msg) {
    int ret = 0, sl;
    int fd;
    while ((fd = open(private_pipe, O_RDONLY | O_NONBLOCK)) == -1 && get_remaining_time() > 0) {}

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    struct timeval tv;
    tv.tv_sec = get_remaining_time();
    tv.tv_usec = 0;
    if ((sl = select(fd + 1, &rfds, NULL, NULL, &tv)) == -1) {
        //perror("select");
        ret = 1;
    } else if (sl > 0) {    // rfds has something to be read
        if (read(fd, msg, sizeof(*msg)) < 0) {
            // perror("read");
            ret = 1;
        } else {
            register_result(*msg);
        }
    } else if (sl == 0) {    // timeout
        register_operation(*msg, GAVUP);
    }

    close(fd);

    return ret;
}

int request_setup(char* private_pipe, Message* msg) {
    snprintf(private_pipe, 100, "/tmp/%i.%lu", getpid(), pthread_self());

    if (mkfifo(private_pipe, 0666) != 0) {
        // perror("mkfifo");
        return 1;
    }

    // pthread_mutex_lock(&g_rid_mutex);
    msg->rid = global_rid;
    global_rid++;
    // pthread_mutex_unlock(&g_rid_mutex);
    msg->pid = getpid();
    msg->tid = pthread_self();

    int upper = 9, lower = 1;
    unsigned r = (unsigned) pthread_self();
    int num = (rand_r(&r) % (upper - lower + 1)) + lower;
    msg->tskload = num;
    msg->tskres = -1;

    return 0;
}

void *request(void* argument) {
    char private_pipe[100];
    Message msg;

    if (request_setup(private_pipe, &msg) != 0) return NULL;
    if (make_request(&msg) == 0) {
        get_result(private_pipe, &msg);
    }

    if (unlink(private_pipe) != 0) {
        perror("unlink");
    }

    return NULL;
}

void close_public_fifo(){
    close(public_pipe_fd);
}

int main(int argc, char* argv[]) {
    if (parse_arguments(argc, argv) != 0) return 1;

    time(&startTime);

    while ((public_pipe_fd = open(public_pipe, O_WRONLY)) == -1 && get_remaining_time() > 0) {}  // does the magic for server closure
    unsigned r = (unsigned) time(NULL);
    int creationSleep = (rand_r(&r) % 9) + 1;
    pthread_t* pid = (pthread_t*) malloc(sizeof(pthread_t));
    while (1) {
        // pthread_mutex_lock(&closd_mutex);
        if (get_remaining_time() == 0 || is_closd)
            break;
        // pthread_mutex_unlock(&closd_mutex);

        usleep(creationSleep * 1000);

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(pid, &attr, request, NULL);
    }

    free(pid);

    //close(public_pipe_fd);

    atexit(close_public_fifo);
    
    pthread_exit(NULL); // -> onto something, not sure
    
    //return 0;
}
