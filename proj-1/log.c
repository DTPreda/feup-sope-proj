#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "log.h"

/**
 * Return 1 if they are different, 0 if they are the same
 */ 
int checkstr(char * str1, char * str2){
    for(int i = 0; str1[i] != '\0'; i++){
        if (str1[i] != str2[i]){
            return 1;
        }
    }
    return 0;
}

void getLog(int argc, char * argv[], char * envp[]){
    for (int i = 0; envp[i] != NULL; ++i){
        if (checkstr("LOG_FILENAME", envp[i]) == 0){
            fprintf(stdout, "Found log_filename: %s\n", envp[i]);
            return;
        }
    }
    fprintf(stdout, "Didnt find log_filename envp\n");
}