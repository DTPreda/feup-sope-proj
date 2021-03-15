#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "log.h"
#include <time.h>
#include <math.h>

#define LOG_FILENAME "LOG_FILENAME"
#define ELDEST_PID "ELDEST_PID"
#define START_TIME "START_TIME"

#define PROC_CREATE 0
#define PROC_EXIT   1
#define SIGNAL_RECV 2
#define SIGNAL_SENT 3
#define FILE_MODF   4


struct timespec start_time, end_time;
long int time_start, time_end;
unsigned int nftot = 0;
unsigned int nfmod = 0;


int xmod(char* in, char* file_name, int verbosity);
__mode_t parse_perms(char* perms, char* filename, int verbosity);
__mode_t get_perms(unsigned int r, unsigned int w, unsigned int x, char op, char target, char* filename);
int recursive_xmod(char* cmd, char* dir_name, int verbosity, int argc, char* argv[]);
char * formatOctal(char *octal);
void strmode(__mode_t mode, char * buf);
long int get_running_time();
void log_start();


/**
 * Setup of environment variables to store the starting time of program
 * and eldest pid
 */
void log_start() {
    
    if (getenv(ELDEST_PID)) {  // if the env variable already exists
        time_start = atol(getenv(START_TIME));
    } 
    else {
        clock_gettime(CLOCK_REALTIME, &start_time);
        time_start = start_time.tv_sec * 1000 + start_time.tv_nsec/(pow(10, 6));    //time in ms

        char pid[15];
        char st_time[50];
        snprintf(pid, sizeof(pid), "%d", getpid());
        snprintf(st_time, sizeof(st_time) , "%ld", time_start);

        int stat = setenv(START_TIME, st_time, 0);           //store the starting time on environment variable
        if (stat == -1) {
            fprintf(stderr, "Error setting environment variable\n");
            exit(1);
        } 

        stat = setenv(ELDEST_PID, pid, 0);                  //store the eldest_pid on environment variable
        if (stat == -1) {
            fprintf(stderr, "Error setting environment variable\n");
            exit(1);
        }

        char* log_file_name = (char*) malloc(sizeof(char) * strlen(getenv(LOG_FILENAME)));
        strcpy(log_file_name, getenv(LOG_FILENAME));

        if(log_file_name){
            FILE* log_file = fopen(log_file_name, "w");
            fclose(log_file);
        }

        free(log_file_name);
    }
}

void write_to_log(unsigned int event, char* info) {
    char* log_file_name = (char*) malloc(sizeof(char) * strlen(getenv(LOG_FILENAME)));
    strcpy(log_file_name, getenv(LOG_FILENAME));

    if (log_file_name) {
        FILE* log_file;

        log_file = fopen(log_file_name, "a");

        char str[100];
        switch (event)
        {
        case PROC_CREATE:
            sprintf(str, "%ld ; %d ; PROC_CREAT ; %s\n", get_running_time() - time_start, getpid(), "1234");
            break;
        case PROC_EXIT:
            sprintf(str, "%ld ; %d ; PROC_EXIT ; %s\n", get_running_time() - time_start, getpid(), "test");
            break;
        case SIGNAL_RECV:
            sprintf(str, "%ld ; %d ; SIGNAL_RECV ; %s\n", get_running_time() - time_start, getpid(), "test");
            break;
        case SIGNAL_SENT:
            sprintf(str, "%ld ; %d ; SIGNAL_SENT ; %s\n", get_running_time() - time_start, getpid(), "test");
            break;
        case FILE_MODF:
            sprintf(str, "%ld ; %d ; FILE_MODF ; %s\n", get_running_time() - time_start, getpid(), "test");
        default:
            break;
        }

        fputs(str, log_file);
        fclose(log_file);
    } else {
        fprintf(stdout, "LOG_FILENAME env variable was not found.");
    }

    free(log_file_name);
}

void sig_handler(int signo) {
    if (signo == SIGINT)
    {
        int option;
        fprintf(stdout,"\nSIGINT RECEIVED. I am the process with a PID of %d\n", getpid());
        killpg(getpgrp(), SIGUSR1);
        wait(-1);
        fprintf(stdout, "Would you wish to proceed? [Y/N]\n");
        option = getchar();
        switch (option)
        {
        case 'Y':
        case 'y':
            fprintf(stdout, "Resuming process \n");
            break;
        case 'N':
        case 'n':
            killpg(getpgrp(), SIGUSR2);
            exit(-1);
        default:
            fprintf(stdout, "Unknown option, aborting program\n");
            killpg(getpgrp(), SIGUSR2);
            exit(-1);
        }
    } else if (signo == SIGUSR1) {
        fprintf(stdout, "%i ; %s ; %i ; %i\n", getpid(), "FILENAME", 0, 0);
        sleep(0.25);
    } else if (signo == SIGUSR2){
        exit(2);
    }
}


/**
 * Gets the time that the process runned until the moment this function is called
 @return double with the time 
*/
long int get_running_time() {
    clock_gettime(CLOCK_REALTIME, &end_time);
    long int delta_ms = end_time.tv_sec * 1000 + end_time.tv_nsec/(pow(10, 6));    //time in ms
    return delta_ms;
}

__mode_t parse_perms(char* perms, char* filename, int verbosity){
    __mode_t ret = 0;
    __mode_t old = 0; //to check if any change was made to the mode
    struct stat stb;
    if(stat(filename, &stb) != 0){	//get permissions
        perror("Stat");
        return __UINT32_MAX__;
    }
    ret = stb.st_mode;
    old = stb.st_mode;

    char* input = strtok(perms, " ");
    for( ; input != NULL; ){
        size_t len = strlen(input);
        char target = 'a';
        char mode;
        unsigned int read = 0, write = 0, execute = 0;
        for(int i = len - 1; input[i] != '+' && input[i] != '-' && input[i] != '=' && i > 0; i--){
            if(input[i] == 'r') read = 1;
            if(input[i] == 'w') write = 1;
            if(input[i] == 'x') execute = 1;
        }

        if(!read && !write && !execute){
            fprintf(stderr, "Invalid input\n");
            return __UINT32_MAX__;
        }

        switch (input[0]){
            case 'u':
                target = 'u';
                mode = input[1];
                break;
            case 'g':
                target = 'g';
                mode = input[1];
                break;
            case 'o':
                target = 'o';
                mode = perms[1];
                break;
            case 'a':
                mode = perms[1];
                break;
            default:
                mode = perms[0];
                break;
        }

        __mode_t temp = 0, sec_temp = 0;
        if(mode == '+'){
            temp = get_perms(read, write, execute, mode, target, filename);
            if(temp == __UINT32_MAX__) return __UINT32_MAX__;
            ret |= temp;
        } else if(mode == '='){

            temp = get_perms(1, 1, 1, '+', target, filename);
            if(temp == __UINT32_MAX__) return __UINT32_MAX__;
            
            sec_temp = get_perms(read, write, execute, '+', target, filename);
            if(sec_temp == __UINT32_MAX__) return __UINT32_MAX__;

            ret &= ~(temp);
            ret |= sec_temp;
        } else {
            temp = get_perms(read, write, execute, mode, target, filename);
            if(temp == __UINT32_MAX__) return __UINT32_MAX__;

            ret &= temp;
        }

        input = strtok(NULL, " ");
    }


    char oldMode[15]; 
    char newMode[15];
    str_mode(old, oldMode);
    str_mode(ret, newMode);

    if (ret == old && verbosity == 1)
        printf("mode of '%s' retained as 0%o (%s)\n", filename, ret % 512, oldMode);

    if (ret != old) {
        if (verbosity)
            printf("mode of '%s' changed from 0%o (%s) to 0%o (%s)\n", filename, old % 512, oldMode, ret % 512, newMode);
    }
    return ret;
}

__mode_t get_perms(unsigned int r, unsigned int w, unsigned int x, char op, char target, char* filename){
    __mode_t ret = 0;

    unsigned int modes[3] = {r, w, x};
    unsigned int targets[3] = {0, 0, 0};
    if(target == 'a'){
        targets[0] = 1; targets[1] = 1; targets[2] = 1;
    } else {
        if(target == 'u') targets[2] = 1;
        if(target == 'g') targets[1] = 1;
        if(target == 'o') targets[0] = 1;
    }
    if(op == '+'){
        //ret = permissões do ficheiro;
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                ret |= modes[i]*targets[j] << ((2 - i) + 3*j);
            }
        }
    } else if (op == '-'){
        //ret = permissões do ficheiro
        struct stat stb;
        if(stat(filename, &stb) != 0){	//get permissions
            perror("Stat");
            return __UINT32_MAX__;
        }
        ret = stb.st_mode;
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                ret &= ~(modes[i]*targets[j] << (((2 - i) + 3*j)));
            }
        }
    }
    return ret;
}

int recursive_xmod(char* cmd, char* dir_name, int verbosity, int argc, char *argv[]){
    //filename points to a dir
    char copy[100];
    char filename[100];
    DIR* d;
    struct dirent *dir;
    d = opendir(dir_name);
    if(d) {
        if(xmod(cmd, dir_name, verbosity) != 0){
            perror("chmod");
            return 1;
        }

        while((dir = readdir(d)) != NULL){
            strcpy(copy, cmd);
            if(dir->d_type == DT_REG || dir->d_type == DT_LNK) { //if it is a regular file
                strcpy(filename, "");
                strcat(filename, dir_name); strcat(filename, "/"); // filename = dir_name/
                strcat(filename, dir->d_name);

                if(xmod(cmd, filename, verbosity) != 0){
                    perror("chmod");
                    return 1;
                }
            } else if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0){
                int pid;
                if((pid = fork()) < 0){
                    perror("fork");
                    return 1;
                }
                if(pid == 0){

                    strcpy(filename, ""); 
                    strcat(filename, dir_name); strcat(filename, "/");
                    strcat(filename, dir->d_name);

                    argv[argc - 1] = filename;
                    if (execv("xmod", argv) == -1) {
                        perror("execve");
                        return 1;
                    }
                }  else {
                    wait(0);
                }
            }
        }
        return 0;
    }

    return 1;
}

/**
 * Converts octal input to format "u=--- g=--- o=---", where "-" can be 'r' 'w' or 'x'
 */
char * format_octal(char *octal){
    char* result = (char *) malloc( 18*sizeof(char));
    strcat(result, "u=");
    for (int i = 1; i < strlen(octal); i++){
        switch (octal[i]){
        case '7':
            strcat(result, "rwx");
            break;
        case '6':
            strcat(result, "rw");
            break;
        case '5':
            strcat(result, "rx");
            break;
        case '4':
            strcat(result, "r");
            break;
        case '3':
            strcat(result, "wx");
            break;
        case '2':
            strcat(result, "w");
            break;
        case '1':
            strcat(result, "x");
            break;
        case '0':
            strcat(result, "");
            break;
        default:
            break;
        }

        if (i == 1){
            strcat(result, " g=");
        } else if (i == 2){
            strcat(result, " o=");
        }
    }
    return result;
}

/**
 * Converts the mode of permissions to format rwxrwxrwx
 */
void str_mode(__mode_t mode, char * buf) {
  const char chars[] = "rwxrwxrwx";
  for (size_t i = 0; i < 9; i++) {
    buf[i] = (mode & (1 << (8-i))) ? chars[i] : '-';
  }
  buf[9] = '\0';
}



int set_handlers(){
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
        perror("signal");
        return 1;
    }
    
    if (signal(SIGUSR2, sig_handler) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    return 0;
}


int get_options(int* verbose, int* recursive, int* index, int argc, char* argv[]){
    int option;
    while ((option = getopt(argc, argv, "vcR")) != -1)
    {
        switch (option) {
            case 'v':
                *verbose = 1;
                break;
            case 'c':
                *verbose = 2;
                break;
            case 'R':
                *recursive = 1;
                break;
            case '?':
                if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option '-%c'.\n", optopt);
                    return 1;
                }
            default:
                return 1;
        }
    }

    *index = optind;

    return 0;
}

void get_input(char* input, char* in, char* file_name, int index, int argc, char* argv[]){
    if (input[0] == '0'){  //get input 
        in = formatOctal(input);
    } else {
        strcpy(in, "");
        for (int i = index; i < argc - 1; i++){ //get all inputs u=rwx g=rx o=wx
            strcat(in, argv[i]);
            strcat(in, " ");
        }
    }
}

int xmod(char* in, char* file_name, int verbosity){
    __mode_t mode;
    if((mode = parse_perms(in, file_name, verbosity)) == __UINT32_MAX__) return 1;
    if(chmod(file_name, mode) != 0){
        perror("chmod");
        return 1;
    }
    return 0;
}


int run_xmod(char* in, char* file_name, int verbosity, int recursive, int argc, char* argv[]){
    struct stat st;
    if(stat(file_name, &st) != 0) {
        perror("stat");
        return 1;
    }

    __mode_t arg_info = st.st_mode;

    if (recursive) {
        if ((arg_info & __S_IFDIR) != 0) {
            if(recursive_xmod(in, file_name, verbosity, argc, argv)){
                return 1;
            }
            return 0;
        } else {
            fprintf(stderr, "Invalid option, not a directory.\n");
            return 1;
        }
    } else {
        nftot += 1;
        if(xmod(in, file_name, verbosity) != 0){
            perror("chmod");
            return 1;
        }
        nfmod += 1;
        return 0;
    }
}

int main(int argc, char* argv[], char* envp[]){
    char exit_code = '0';

    // verificar se há argumentos suficientes para correr o programa
    if(argc <= 2){
        fprintf(stdout, "Invalid number of arguments\n");
        exit_code = '1';
    }
    
    log_start();
    
    if(set_handlers()){
        exit_code = '1';
    } else {
        write_to_log(0, "argv");
        

        int verbosity, recursive, index;
        if(get_options(&verbosity, &recursive, &index, argc, argv)){
            exit_code = 1;
        } else {
            char *input = argv[index];
            char *in = (char *) malloc (18 * sizeof(char));
            char *file_name = argv[argc - 1];
            get_input(input, in, file_name, index, argc, argv);

            if(run_xmod(in, file_name, verbosity, recursive, argc, argv) != 0){
                exit_code = '1';
            }
        }
    }

    /*write_to_log(PROC_EXIT, &exit_code);
    if(getpid() == atoi(getenv(ELDEST_PID))) wait(0);
    return atoi(exit_code);
    */
   return 0;
}

