#include "register.h"

void register_op(Message msg, int type){
    time_t t;
    if((t = time(NULL)) == -1){
        perror("time");
        return;
    }
    fprintf(stdout, "%u ; %i ; %i ; %i ; %lu ; %i ; ", t, msg.rid, msg.tskload, msg.pid, msg.tid, msg.tskres);
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

}