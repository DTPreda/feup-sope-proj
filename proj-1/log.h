#ifndef _LOG_H_
#define _LOG_H_

/**
 * @brief Setup of environment variables to store the starting time of program
 * and eldest pid.
 * 
 * @return 0 on success, 1 otherwise.
 */
int log_start();

/**
 * @brief Writes to the log file specified in the LOG_FILENAME environment
 * variable some info regarding the execution of the program.
 * 
 * @param event Type of event to write.
 * 
 * @param info Specific info regarding the event.
 */ 
void write_to_log(unsigned int event, char* info);

/**
 * @brief Calculates the time of execution since the start of the program,
 * in milliseconds.
 * 
 * @return Time since the start of execution, in milliseconds.
 */ 
long int get_running_time();


#endif  // _LOG_H_
