#include "register.h"

pthread_mutex_t output_controller;

void register_op(Message msg, int type){
    time_t t;
    if((t = time(NULL)) == -1){
        perror("time");
        return;
    }
    pthread_mutex_lock(&output_controller);        
    fprintf(stdout, "%lu ; %i ; %i ; %i ; %lu ; %i ; ", t, msg.rid, msg.tskload, msg.pid, msg.tid, msg.tskres);
    switch(type){
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
    pthread_mutex_unlock(&output_controller);
}