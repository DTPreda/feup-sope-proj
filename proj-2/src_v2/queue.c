#include "headers/queue.h"

int queue_init(message_queue* q, int size) {
    q->arr = (Message*) malloc(sizeof(Message) * size);
    if (q->arr == NULL) {
        return 1;
    }

    q->order = (int*) malloc(sizeof(int) * size);
    if (q->order == NULL) {
        return 1;
    }

    q->size = size;
    q->full = 0;

    for (int i = 0; i < q->size; i++) {
        q->order[i] = -1;
    }

    return 0;
}

int queue_push(message_queue* q, Message val) {
    if (q->full != q->size) {
        for (int i = 0; i < q->size; i++) {
            if (q->order[i] == -1) {
                q->arr[i] = val;
                q->order[i] = q->full;
                break;
            }
        }
        q->full++;
        return 0;
    }
    return 1;
}

int queue_pop(message_queue* q, Message* ret) {
    if (q->full != 0) {
        for (int i = 0; i < q->size; i++) {
            if (q->order[i] == 0) {
                *ret = q->arr[i];
                q->order[i] = -1;
                break;
            }
        }
        for (int i = 0; i < q->size; i++) {
            if (q->order[i] != -1) {
                q->order[i]--;
            }
        }
        q->full--;
        return 0;
    }
    return 1;
}

void queue_destroy(message_queue* q) {
    free(q->arr);
    free(q->order);
}

int queue_is_empty(message_queue* q) {
    return q->full == 0;
}

void print_order(message_queue* q) {
    for (int i = 0; i < q->size; i++) {
        fprintf(stdout, "%i, ", q->order[i]);
    }
    fprintf(stdout, "\n");
}

void print_arr(message_queue* q) {
    for (int i = 0; i < q->size; i++) {
        if (q->order[i] != -1) {
            fprintf(stdout, "%i, ", q->arr[i].rid);
        } else {
            fprintf(stdout, "-1, ");
        }
    }
    fprintf(stdout, "\n");
}