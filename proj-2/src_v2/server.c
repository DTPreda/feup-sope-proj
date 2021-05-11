#include "server.h"


int input_time;
int start_time;

message_queue buffer;
int buffer_size = 10;

char* public_fifo;
int public_fifo_fd;

pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_access = PTHREAD_MUTEX_INITIALIZER;

sem_t empty;
sem_t full;
sem_t curr_n_threads;

int get_remaining_time(){
    time_t current_time = time(NULL);
    time_t ret = input_time - current_time + start_time;
    if(ret < 0) return 0;
    return ret;
}

void register_operation(Message* msg, int type) {
    time_t t = time(NULL);
    if (t == -1) return;

    pthread_mutex_lock(&output_mutex);
    fprintf(stdout, "%li ; %i ; %i ; %i ; %lu ; %i ; ", t, msg->rid, msg->tskload, msg->pid , (unsigned long) msg->tid, msg->tskres);
    switch (type) {
        case(RECVD):
            fprintf(stdout, "RECVD");
            break;
        case(TSKEX):
            fprintf(stdout, "TSKEX");
            break;
        case(TSKDN):
            fprintf(stdout, "TSKDN");
            break;
        case(TOOLATE):
            fprintf(stdout, "2LATE");
            break;
        case(FAILD):
            fprintf(stdout, "FAILD");
            break;
        default:
            break;
    }
    fprintf(stdout, "\n");
    pthread_mutex_unlock(&output_mutex);
}

int parse_arguments(int argc, char* argv[]){
    if (argc != 4 && argc != 6) return 1;
    if (argc == 6) {
        if(strncmp(argv[3], "-l", 2) != 0) return 1;

        buffer_size = atoi(argv[4]);
        if(buffer_size == 0) return 1;

        public_fifo = argv[5];
    } else {
        public_fifo = argv[3];
    }

    if (strncmp(argv[1], "-t", 2) != 0) return 1;
    
    input_time = atoi(argv[2]);
    if(input_time == 0) return 1;
    return 0;
}

int set_server_up(message_queue* q, int bufsz, char* fifo_name) {
    if (queue_init(q, buffer_size) != 0) {
        fprintf(stderr, "queue_init: Unable to initialize queue.\n");
        return 1;
    } 
    
    if (mkfifo(fifo_name, 0666) != 0) {
        perror("mkfifo");
        return 1;
    }

    if (sem_init(&empty, 0, q->size) != 0 || sem_init(&full, 0, 0) != 0 || sem_init(&curr_n_threads, 0, 0) != 0) {
        perror("sem_init");
        return 1;
    }

    if ((public_fifo_fd = open(fifo_name, O_RDONLY)) == -1) {
        perror("open");
        return 1;
    }

    return 0;
}

int get_request(Message* msg){
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(public_fifo_fd, &rfds);

    struct timeval tv;
    tv.tv_sec = get_remaining_time();
    tv.tv_usec = 0;

    int sl = -1;
    if((sl = select(public_fifo_fd + 1, &rfds, NULL, NULL, &tv)) == -1) {
        perror("select");
        return 1;
    } else if (sl > 0) {
        int r = read(public_fifo_fd, msg, sizeof(Message));
        if(r == -1) {
            perror("read");
            return 1;
        } else if (r == 0){
            return 1;
        }
        register_operation(msg, RECVD);
    } else if (sl == 0) {
        return 1;
    }

    return 0;
}

void free_resources() {
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&curr_n_threads);

    pthread_mutex_destroy(&output_mutex);
    pthread_mutex_destroy(&queue_access);
    
    queue_destroy(&buffer);
}

void close_fifo() {
    close(public_fifo_fd);

    if (unlink(public_fifo) != 0) {
        perror("unlink");
    }
}

void insert_item(Message* msg, message_queue* buffer) {
    if (sem_wait(&empty) != 0) {
        perror("sem_wait");
        return;
    }

    pthread_mutex_lock(&queue_access);
    queue_push(buffer, *msg);
    pthread_mutex_unlock(&queue_access);

    if (sem_post(&full) != 0) {
        perror("sem_post");
        return;
    }
}

void *attend_request(void* argument) {
    sem_post(&curr_n_threads);
    Message* msg = (Message*) argument;
    
    if (get_remaining_time() != 0) {
        msg->tskres = task(msg->tskload);
        register_operation(msg, TSKEX);
    }

    insert_item(msg, &buffer);
    free(msg);

    sem_trywait(&curr_n_threads);

    return NULL;
}

void send_result(Message* msg) {
    char private_fifo[100];
    snprintf(private_fifo, 100, "/tmp/%i.%lu", msg->pid, (unsigned long) msg->tid);

    int fd;
    if ((fd = open(private_fifo, O_WRONLY | O_NONBLOCK)) == -1) {
        register_operation(msg, FAILD);
    } else {
        if (write(fd, msg, sizeof(Message)) == -1) {
            register_operation(msg, FAILD);
        } else {
            if (msg->tskres == -1) {
                register_operation(msg, TOOLATE);
            } else {
                register_operation(msg, TSKDN);
            }
        }
    }

    close(fd);
}

void *consumer_thread(void* argument){
    Message msg;
    while (1) {
        if (sem_wait(&full) != 0) {
            continue;
        }

        pthread_mutex_lock(&queue_access);
        queue_pop(&buffer, &msg);
        pthread_mutex_unlock(&queue_access);

        if (sem_post(&empty) != 0) {
            perror("sem_post");
        }

        if (msg.tskres == POISON_PILL) {
            //printf("Found poison pill\n");
            break;
        }
        
        send_result(&msg);
    }
    int val;
    sem_getvalue(&curr_n_threads, &val);
    while (!queue_is_empty(&buffer) || val != 0) {


        if (sem_wait(&full) != 0) {
            continue;
        }

        pthread_mutex_lock(&queue_access);
        queue_pop(&buffer, &msg);
        pthread_mutex_unlock(&queue_access);

        if (sem_post(&empty) != 0) {
            perror("sem_post");
        }
        
        send_result(&msg);
        
        sem_getvalue(&curr_n_threads, &val);
    }
    fprintf(stdout, "Consumer thread terminating.\n");
    return NULL;
}

int set_up_consumer_thread(pthread_t* pid, pthread_attr_t* attr) {
    pthread_attr_init(attr);
    pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);

    if (pthread_create(pid, attr, consumer_thread, NULL) != 0) {
        return 1;
    }

    return 0;
}

void poison_pill() {
    Message msg;
    msg.tskres = POISON_PILL;

    sem_wait(&empty);

    pthread_mutex_lock(&queue_access);
    queue_push(&buffer, msg);
    pthread_mutex_unlock(&queue_access);
    
    sem_post(&full);
}

int main(int argc, char* argv[]) {
    if (parse_arguments(argc, argv) != 0) {
        fprintf(stdout, "Correct usage: s <-t nsecs> [-l bufsz] <fifoname>\n");
        return 1;
    }

    if ((start_time = time(NULL)) == -1) {
        fprintf(stdout, "Impossible to get start time.\n");
        return 1;
    }

    if (set_server_up(&buffer, buffer_size, public_fifo) != 0) {
        fprintf(stdout, "Unable to set up crucial server resources.\n");
        return 1;
    }

    pthread_t pid;
    pthread_attr_t attr;

    if (set_up_consumer_thread(&pid, &attr) != 0) {
        fprintf(stdout, "Unable to set up consumer thread.\n");
        return 1;
    }
    while (get_remaining_time() != 0) {
        Message* msg = (Message*) malloc(sizeof(Message));
        if (get_request(msg) != 0) {
            continue;
        }
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&pid, &attr, attend_request, (void*) msg);
    }

    pthread_attr_destroy(&attr);

    close_fifo();

    atexit(free_resources);

    fprintf(stdout, "Reader thread terminating.\n");
    
    poison_pill();

    pthread_exit(NULL);

    //return 0;
}