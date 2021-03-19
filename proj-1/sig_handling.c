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

void sig_handler(int signo) {
    char sig_received[15];
    snprintf(sig_received, sizeof(sig_received), "%i", signo);
    write_to_log(SIGNAL_RECV, sig_received);

    if (signo == SIGINT) {
        fprintf(stdout, "%i ; %s ; %i ; %i\n", getpid(), curr_file, nftot, nfmod);
        // First process sends signals to the rest of the group
        if (getpid() == FIRST_PROCESS_PID) {
            int option;
            char msg1[15], msg2[15];
            snprintf(msg1, sizeof(msg1), "SIGUSR1 : %d", getpid());
            snprintf(msg2, sizeof(msg2), "SIGCONT : %d", getpid());

            sleep(0.25);
            fprintf(stdout, "Would you wish to proceed? [Y/N]\n");
            while ( (option = getchar()) == '\n') {}

            write_to_log(SIGNAL_SENT, msg2);
            killpg(getpgrp(), SIGCONT);

            switch (option) {
                case 'Y':
                case 'y':
                    fprintf(stdout, "Resuming process \n");
                    break;
                case 'N':
                case 'n':
                    write_to_log(SIGNAL_SENT, msg1);
                    killpg(getpgrp(), SIGUSR1);
                    break;
                default:
                    fprintf(stdout, "Unknown option, aborting program\n");
                    write_to_log(SIGNAL_SENT, msg1);
                    killpg(getpgrp(), SIGUSR1);
                    break;
            }
        } else {
            pause();
        }
    } else if (signo == SIGUSR1) {
        if (getpid() == FIRST_PROCESS_PID) wait(0);
        write_to_log(PROC_EXIT, "1");
        exit(1);
    }
}


int set_handlers() {
    for (int i = 1; i < 32; i++) {
        if (i != SIGKILL && i != SIGSTOP) {
            if (signal(i, sig_handler) == SIG_ERR) {
                fprintf(stdout, "%i\n", i);
                perror("signal");
                return 1;
            }
        }
    }
    return 0;
}
