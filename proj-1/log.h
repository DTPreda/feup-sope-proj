#ifndef _LOG_H_
#define _LOG_H_

int log_start();
void write_to_log(unsigned int event, char* info);
long int get_running_time();


#endif //_LOG_H_
