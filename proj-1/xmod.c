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


struct timespec startTime, endTime;
long int timeStart, timeEnd;
static char* log_file;
unsigned int nftot = 0;
unsigned int nfmod = 0;


int xmod(char* in, char* file_name, int verbosity);
__mode_t parse_perms(char* perms, char* filename, int verbosity);
__mode_t get_perms(unsigned int r, unsigned int w, unsigned int x, char op, char target, char* filename);
void chmod_dir(char* cmd, char* dir_name, int verbosity, int argc, char* argv[], char* envp[]);
char * formatOctal(char *octal);
void strmode(__mode_t mode, char * buf);
long int get_running_time();
void set_log_file(FILE* logFile);
void log_start();


/**
 * Gets the log_filename output file into log_file global variable
 * Opens the file of log_filename.
 *@param FILE* variable which will have the file of log_filename
 */
void set_log_file(FILE* logFile) {
    log_file = getenv(LOG_FILENAME);

    if (log_file) {
        if (atoi(getenv(ELDEST_PID)) != getpid()) {     //is not the eldest process, so add content to file
            logFile = fopen(log_file, "a");
        } 
        else {              //is the eldest so we have to truncate
            logFile = fopen(log_file, "w");  // erase content
            logFile = freopen(log_file, "a", logFile);    //reopen to add content
        }
    }
}

/**
 * Setup of environment variables to store the starting time of program
 * and eldest pid
 */
void log_start() {
    
    if (getenv(ELDEST_PID)) {  // if the env variable already exists
        //fprintf(stdout, "ELDEST_PIT: %d, START_TIME: %ld, LOG_FILENAME: %s\n", atoi(getenv(ELDEST_PID)), atol(getenv(START_TIME)), getenv(LOG_FILENAME));
        timeStart = atol(getenv(START_TIME));
    } 
    else {
        //fprintf(stdout, "setting up env variable\n");
        clock_gettime(CLOCK_REALTIME, &startTime);
        long int timeStart = startTime.tv_sec * 1000 + startTime.tv_nsec/(pow(10, 6));    //time in ms

        char pid[15];
        char stTime[50];
        snprintf(pid, sizeof(pid), "%d", getpid());
        snprintf(stTime, sizeof(stTime) , "%ld", timeStart);

        int stat = setenv(START_TIME, stTime, 0);           //store the starting time on environment variable
        if (stat == -1) {
            fprintf(stderr, "Error setting environment variable\n");
            exit(1);
        } 
        //fprintf(stdout, "Created start_time env variable: %s\n", getenv(START_TIME));

        stat = setenv(ELDEST_PID, pid, 0);                  //store the eldest_pid on environment variable
        if (stat == -1) {
            fprintf(stderr, "Error setting environment variable\n");
            exit(1);
        }
        //fprintf(stdout, "Created eldest_pid env variable: %s\n", getenv(ELDEST_PID));
    }
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

void write_to_log(unsigned int event, char* info) {
    if (*log_file == NULL)
        return;

     FILE* fd = fopen(log_file, "w");
     char str[100];
     switch (event)
     {
     case 0:
         sprintf(str, "%f ; %d ; PROC_CREAT ; %s\n", getRunningTime(), getpid(), "1234");
         break;
     case 1:
        sprintf(str, "%f, %d ; PROC_EXIT ; %s\n", getRunningTime(), getpid(), "test");
        break;
    case 2:
        sprintf(str, "%f ; %d ; SIGNAL_RECV ; %s\n", getRunningTime(), getpid(), "test");
        break;
    case 3:
        sprintf(str, "%f ; %d ; SIGNAL_SENT ; %s\n", getRunningTime(), getpid(), "test");
        break;
    case 4:
        sprintf(str, "%f ; %d ; FILE_MODF ; %s\n", getRunningTime(), getpid(), "test");
     default:
         break;
     }

    fputs(str, fd);
    fclose(fd);
}

/**
 * Gets the time that the process runned until the moment this function is called
 @return double with the time 
*/
long int get_running_time() {
    //sleep(1);
    clock_gettime(CLOCK_REALTIME, &endTime);
    long int delta_ms = startTime.tv_sec * 1000 + startTime.tv_nsec/(pow(10, 6));    //time in ms
    //fprintf(stdout, "Elapsed time(ms): %f", delta_ms);
    return delta_ms;
}

__mode_t parse_perms(char* perms, char* filename, int verbosity){
    __mode_t ret = 0;
    __mode_t old = 0; //to check if any change was made to the mode
    struct stat stb;
    if(stat(filename, &stb) != 0){	//get permissions
        perror("Stat");
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
            exit(-1);
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

        if(mode == '+'){
            ret |= get_perms(read, write, execute, mode, target, filename);
        } else if(mode == '='){
            ret &= ~(get_perms(1, 1, 1, '+', target, filename));
            ret |= get_perms(read, write, execute, '+', target, filename);
        } else {
            ret &= get_perms(read, write, execute, mode, target, filename);
        }

        input = strtok(NULL, " ");
    }


    char oldMode[15]; 
    char newMode[15];
    strmode(old, oldMode);
    strmode(ret, newMode);

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

void chmod_dir(char* cmd, char* dir_name, int verbosity, int argc, char *argv[], char*envp[]){
    //filename points to a dir
    char copy[100];
    char filename[100];
    DIR* d;
    struct dirent *dir;
    d = opendir(dir_name);
    if(d) {
        if(xmod(cmd, dir_name, verbosity) != 0){
            perror("chmod");
            exit(1);
        }

        while((dir = readdir(d)) != NULL){
            strcpy(copy, cmd);
            if(dir->d_type == DT_REG || dir->d_type == DT_LNK) { //if it is a regular file
                strcpy(filename, "");
                strcat(filename, dir_name); strcat(filename, "/"); // filename = dir_name/
                strcat(filename, dir->d_name);

                if(xmod(cmd, filename, verbosity) != 0){
                    perror("chmod");
                }
            } else if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0){
                int pid;
                if((pid = fork()) < 0){
                    perror("fork");
                    return;
                }
                if(pid == 0){

                    strcpy(filename, ""); 
                    strcat(filename, dir_name); strcat(filename, "/");
                    strcat(filename, dir->d_name);

                    argv[argc - 1] = filename;
                    if (execv("xmod", argv) == -1)
                        perror("execve");
                    return chmod_dir(copy, filename, verbosity, argc, argv, envp); //just runs if error on execve
                } else {
                    wait(0);
                }
            }
        }
    }
}

/**
 * Converts octal input to format "u=--- g=--- o=---", where "-" can be 'r' 'w' or 'x'
 */
char * formatOctal(char *octal){
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
void strmode(__mode_t mode, char * buf) {
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


void get_options(int* verbose, int* recursive, int* index, int argc, char* argv[]){
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
                    exit(-1);
                }
            default:
                abort();
        }
    }

    *index = optind;
}

void get_input(char* input, char* in, char* file_name, int index, int argc, char* argv[]){
    if (input[0] == '0'){  //get input 
        in = formatOctal(input);
    } else {
        for (int i = index; i < argc - 1; i++){ //get all inputs u=rwx g=rx o=wx
            strcat(in, argv[i]);
            strcat(in, " ");
            //printf("String: %s\n", in);
        }
    }
}

int xmod(char* in, char* filename, int verbosity){
    __mode_t mode = parse_perms(in, filename, verbosity);
        if(chmod(filename, mode) != 0){
            perror("chmod");
            return 1;
        }
    return 0;
}


int run_xmod(char* in, char* filename, int verbosity, int recursive, int argc, char* argv[], char* envp[]){
    struct stat st;
    if(stat(filename, &st) != 0) {
        perror("stat");
        return 1;
    }

    __mode_t arg_info = st.st_mode;

    if (recursive) {
        if ((arg_info & __S_IFDIR) != 0) {
            chmod_dir(in, filename, verbosity, argc, argv, envp);
            return 0;
        } else {
            fprintf(stderr, "Invalid option, not a directory.\n");
            return 1;
        }
    } else {
        nftot += 1;
        if(xmod(in, filename, verbosity) != 0){
            perror("chmod");
            return 1;
        }
        nfmod += 1;
        return 0;
    }
}

int main(int argc, char* argv[], char* envp[]){
        
    FILE* logFile;

    log_start();
    set_log_file(logFile);


    if(argc <= 2){
        fprintf(stdout, "Invalid number of arguments\n");
        exit(1);
    }


    if(set_handlers()){
        return 1;
    }

    log_file = getenv("LOG_FILENAME");
    write_to_log(0, "argv");
    
    int verbosity, recursive, index;
    get_options(&verbosity, &recursive, &index, argc, argv);
    
    char *input = argv[index];
    char *in = (char *) malloc (18 * sizeof(char));
    char *filename = argv[argc - 1];
    get_input(input, in, filename, index, argc, argv);

    if(run_xmod(in, filename, verbosity, recursive, argc, argv, envp) != 0){
        return 1;
    }
    
    double value = get_running_time();
    fprintf(stdout, "RUNNING TIME: %f\n", value);
    return 0;
}

