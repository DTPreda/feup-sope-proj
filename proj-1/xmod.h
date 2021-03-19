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



/**
 * @brief Call recursive or single file xmod according to the file or directory specified
 * 
 * @param in input by user
 * 
 * @param file_name file to be changed
 * 
 * @param verbosity verbosity flag
 * 
 * @param recursive recursive flag
 * 
 * @param argc number of args
 * 
 * @param argv args
 * 
 * @return 0 if successfull, 1 otherwise
 */ 
int run_xmod(char* in, char* file_name, int verbosity, int recursive, int argc, char* argv[]);


/**
 * @brief Parses and changes the permissions of a single file
 * introduced by user 
 * 
 * @param in input(permissions)
 * 
 * @param file_name file to change
 * 
 * @param verbosity verbosity flag
 * 
 * @return 0 if successfull, 1 otherwise
 */ 
int xmod(char* in, char* file_name, int verbosity);

/**
 * @brief recursive function that changes(or mantains) the permissions of the file
 * mentioned in dir_name
 * 
 * @param cmd command line input by user
 * 
 * @param dir_name name of directory to change permissions
 * 
 * @param verbosity flag that says if user want verbosity output
 * 
 * @param argc number of args
 * 
 * @param argv args
 * 
 * @return 0 if successfull, 1 otherwise 
 */ 
int recursive_xmod(char* cmd, char* dir_name, int verbosity, int argc, char* argv[]);


/**
 * @brief Concatenates two strings on the format dir/file_name
 * 
 * @param dir first string
 * 
 * @param file_name second string to concatenate
 * 
 * @param ret string with result of concatenation
 */ 
void concatenate_dir_file(char* dir, char* file_name, char* ret);

#endif  // XMOD_H_
