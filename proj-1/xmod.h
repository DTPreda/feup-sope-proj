#ifndef XMOD_H_
#define XMOD_H_

#define PROC_CREATE 0
#define PROC_EXIT   1
#define SIGNAL_RECV 2
#define SIGNAL_SENT 3
#define FILE_MODF   4

#define LOG_FILENAME "LOG_FILENAME"
#define FIRST_PROCESS_PID getpgrp()
#define START_TIME "START_TIME"


int xmod(char* in, char* file_name, int verbosity);
int recursive_xmod(char* cmd, char* dir_name, int verbosity, int argc, char* argv[]);
void str_mode(__mode_t mode, char* buf);
void print_changes(__mode_t new_mode, __mode_t old_mode, int verbosity, char* file_name);


#endif  // XMOD_H_
