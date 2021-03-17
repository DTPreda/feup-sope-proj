#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>


#include "./xmod.h"
#include "./sig_handling.h"
#include "./log.h"

unsigned int nftot = 0;
unsigned int nfmod = 0;
char* curr_file;  // currently FILE/DIR passed to argv

/**
 * Global Handler of signals, it writes into a file the information 
 * about the signal received
 */ 
void sig_handler(int signo) {
    if (signo == SIGINT) {
        char sig_received[15];
        snprintf(sig_received, sizeof(sig_received), "SIGINT");
        write_to_log(SIGNAL_RECV, sig_received);

        fprintf(stdout, "%i ; %s ; %i ; %i\n", getpid(), curr_file, nftot, nfmod);
        // The eldest controls the signals
        if (getpid() == FIRST_PROCESS_PID) {
            sleep(0.25);
            int option;
            char msg1[15], msg2[15];
            snprintf(msg1, sizeof(msg1), "SIGUSR1 : %d", getpid());
            snprintf(msg2, sizeof(msg2), "SIGCONT : %d", getpid());

            fprintf(stdout, "Would you wish to proceed? [Y/N]\n");
            while ( (option = getchar()) == '\n') {}
            switch (option) {
                case 'Y':
                case 'y':
                    fprintf(stdout, "Resuming process \n");
                    write_to_log(SIGNAL_SENT, msg2);
                    killpg(getpgrp(), SIGCONT);
                    break;
                case 'N':
                case 'n':
                    write_to_log(SIGNAL_SENT, msg2);
                    killpg(getpgrp(), SIGCONT);
                    write_to_log(SIGNAL_SENT, msg1);
                    killpg(getpgrp(), SIGUSR1);
                    break;
                default:
                    write_to_log(SIGNAL_SENT, msg2);
                    killpg(getpgrp(), SIGCONT);
                    fprintf(stdout, "Unknown option, aborting program\n");
                    write_to_log(SIGNAL_SENT, msg1);
                    killpg(getpgrp(), SIGUSR1);
                    break;
            }
        } else {
            char msg[15];
            snprintf(msg, sizeof(msg), "SIGSTOP : %d", getpid());
            write_to_log(SIGNAL_SENT, msg);
            kill(getpid(), SIGSTOP);

            char sig_received[15];
            snprintf(sig_received, sizeof(sig_received), "SIGCONT");
            write_to_log(SIGNAL_RECV, sig_received);
        }
    } else if (signo == SIGUSR1) {
        char sig_received[15];
        snprintf(sig_received, sizeof(sig_received), "SIGUSR1");
        write_to_log(SIGNAL_RECV, sig_received);

        if (getpid() == FIRST_PROCESS_PID) wait(0);
        write_to_log(PROC_EXIT, "1");
        exit(1);
    }
}



int set_handlers() {
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    return 0;
}


