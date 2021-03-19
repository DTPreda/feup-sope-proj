#ifndef SIG_HANDLING_H_
#define SIG_HANDLING_H_

/**
 * @brief Global Handler of signals, it writes into a file the information 
 * about the signal received
 * 
 * @param signo Number of the signal received, specified in <signal.h>
 */ 
void sig_handler(int signo);

/**
 * @brief Enables the writing of every possible signal, except SIGSTOP and SIGKILL, to the log file
 * specified in LOG_FILENAME
 * 
 * @return 0 if successfull, 1 otherwise
 */ 
int set_handlers();

#endif  // SIG_HANDLING_H
