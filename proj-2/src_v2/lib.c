#include "lib.h"

int result = 0;

int get_result(Message* msg){
    if(msg == NULL){
        return 1;
    }    
    else {
        msg->tskres = result;
        result += 10;
    }
    return 0;
}