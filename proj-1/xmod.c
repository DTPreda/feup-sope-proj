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
#define FIRST_PROCESS_PID getpgrp()
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
char* curr_file;    //currently FILE/DIR passed to argv, needed to use in sig_handler


int xmod(char* in, char* file_name, int verbosity);
__mode_t parse_perms(char* perms, char* filename, int verbosity);
__mode_t get_perms(unsigned int r, unsigned int w, unsigned int x, char op, char target, char* filename);
int recursive_xmod(char* cmd, char* dir_name, int verbosity, int argc, char* argv[]);
char* format_octal(char* octal);
void str_mode(__mode_t mode, char* buf);
long int get_running_time();
int log_start();
void format_argv(int argc, char* argv[], char* str);
int parse_argv(int argc, char* argv[]);
void print_changes(__mode_t new_mode, __mode_t old_mode, int verbosity, char* file_name);


/**
 * Setup of environment variables to store the starting time of program
 * and eldest pid
 */
int log_start() {
    
    if (getpid() != FIRST_PROCESS_PID) {  // if the process is not the first one to be created => START_TIME will already have been created
        time_start = atol(getenv(START_TIME));
    } 
    else {
        clock_gettime(CLOCK_REALTIME, &start_time);
        time_start = start_time.tv_sec * 1000 + start_time.tv_nsec/(pow(10, 6));    //time in ms

        char st_time[50];
        snprintf(st_time, sizeof(st_time) , "%ld", time_start);

        int stat = setenv(START_TIME, st_time, 0);           //store the starting time on environment variable
        if (stat == -1) {
            fprintf(stderr, "Error setting environment variable\n");
            return 1;
        } 

        char* log_file_name = getenv(LOG_FILENAME);
        
        if(log_file_name != NULL){
            FILE* log_file = fopen(log_file_name, "w");
            fclose(log_file);
        }

    }

    return 0;
}

void write_to_log(unsigned int event, char* info) {
    char* log_file_name = getenv(LOG_FILENAME);
    if (log_file_name != NULL) {
        FILE* log_file;

        log_file = fopen(log_file_name, "a");

        char str[100];
        switch (event)
        {
        case PROC_CREATE:
            sprintf(str, "%ld ; %d ; PROC_CREAT ;%s\n", get_running_time() - time_start, getpid(), info);
            break;
        case PROC_EXIT:
            sprintf(str, "%ld ; %d ; PROC_EXIT ; %s\n", get_running_time() - time_start, getpid(), info);
            break;
        case SIGNAL_RECV:
            sprintf(str, "%ld ; %d ; SIGNAL_RECV ; %s\n", get_running_time() - time_start, getpid(), info);
            break;
        case SIGNAL_SENT:
            sprintf(str, "%ld ; %d ; SIGNAL_SENT ; %s\n", get_running_time() - time_start, getpid(), info);
            break;
        case FILE_MODF:
            sprintf(str, "%ld ; %d ; FILE_MODF ; %s\n", get_running_time() - time_start, getpid(), info);
        default:
            break;
        }

        fputs(str, log_file);
        fclose(log_file);
    }
}

/**
 * Global Handler of signals, it writes into a file the information 
 * about the signal received
 */ 
void sig_handler(int signo) {

    if (signo == SIGINT) {
        char sig_received[15];
        sprintf(sig_received, "SIGINT");
        write_to_log(SIGNAL_RECV, sig_received);
        
        fprintf(stdout, "%i ; %s ; %i ; %i\n", getpid(), curr_file, nftot, nfmod);    

        if(getpid() == FIRST_PROCESS_PID){ //The eldest controls the signals
            sleep(0.25);
            int option;
            char msg1[15], msg2[15];
            sprintf(msg1, "SIGUSR1 : %d", getpid());
            sprintf(msg2, "SIGCONT : %d", getpid());

            fprintf(stdout, "Would you wish to proceed? [Y/N]\n");
            while( (option = getchar()) == '\n') ;
            switch (option){
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
        } 
        else {
            char msg[15];
            sprintf(msg, "SIGSTOP : %d", getpid());
            write_to_log(SIGNAL_SENT, msg);
            kill(getpid(), SIGSTOP);
            
            char sig_received[15];
            sprintf(sig_received, "SIGCONT");
            write_to_log(SIGNAL_RECV, sig_received);
        }
    }
    else if (signo == SIGUSR1) {
        char sig_received[15];
        sprintf(sig_received, "SIGUSR1");
        write_to_log(SIGNAL_RECV, sig_received);

        if(getpid() == FIRST_PROCESS_PID) wait(0);
        write_to_log(PROC_EXIT, "1");
        exit(1);
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
    struct stat stb;
    if(stat(filename, &stb) != 0){	//get permissions
        perror("Stat");
        return __UINT32_MAX__;
    }
    ret = stb.st_mode;

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

void concatenate_dir_file(char* dir, char* file_name, char* ret){
    strcpy(ret, "");
    strcat(ret, dir); strcat(ret, "/"); // filename = dir_name/
    strcat(ret, file_name);
}

int recursive_xmod(char* cmd, char* dir_name, int verbosity, int argc, char *argv[]){
    //sleep(2);
    char copy[100];
    char file_name[100];
    DIR* d;
    struct dirent *dir;
    d = opendir(dir_name);
    if(d) {

        if(xmod(cmd, dir_name, verbosity) != 0){
            perror("chmod");
            return 1;
        }

        nftot += 1; // found dir called by argument

        while((dir = readdir(d)) != NULL){
            
            strcpy(copy, cmd);
            if(dir->d_type == DT_REG) { //if it is a regular file
                nftot += 1; // found a file inside the directory
                
                concatenate_dir_file(dir_name, dir->d_name, file_name);

                if(xmod(cmd, file_name, verbosity) != 0){
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

                    concatenate_dir_file(dir_name, dir->d_name, file_name);

                    argv[argc - 1] = file_name;
                    if (execv("xmod", argv) == -1) {
                        argv[argc - 1] = dir_name;
                        perror("execve");
                        return 1;
                    }
                    argv[argc - 1] = dir_name;
                }  
                else {
                    wait(0);
                }
            } else if (dir->d_type == DT_LNK){
                concatenate_dir_file(dir_name, dir->d_name, file_name);
                print_changes(0, 0, 3 & (3 * (verbosity == 1)), file_name);
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
        in = format_octal(input);
    } else {
        strcpy(in, "");
        for (int i = index; i < argc - 1; i++){ //get all inputs u=rwx g=rx o=wx
            strcat(in, argv[i]);
            strcat(in, " ");
        }
    }
}

void print_changes(__mode_t new_mode, __mode_t old_mode, int verbosity, char* file_name){
    if(verbosity == 3){
        printf("neither symbolic link '%s' nor referent has been changed\n", file_name);
        return;
    }

    char old_mode_str[15]; 
    char new_mode_str[15];
    str_mode(old_mode, old_mode_str);
    str_mode(new_mode, new_mode_str);

    if (new_mode == old_mode && verbosity == 1)
        printf("mode of '%s' retained as 0%o (%s)\n", file_name, new_mode % 512, old_mode_str);
    else if (new_mode != old_mode && verbosity)
        printf("mode of '%s' changed from 0%o (%s) to 0%o (%s)\n", file_name, old_mode % 512, old_mode_str, new_mode % 512, new_mode_str);
}

int xmod(char* in, char* file_name, int verbosity){
    __mode_t old_mode = 0;
    
    struct stat stb;
    if(stat(file_name, &stb) != 0){	//get permissions
        perror("Stat");
        return 1;
    }
    old_mode = stb.st_mode;
    
    __mode_t mode = 0;
    if((mode = parse_perms(in, file_name, verbosity)) == __UINT32_MAX__) return 1;
    if(mode != old_mode){
        if(chmod(file_name, mode) != 0){
            perror("chmod");
            return 1;
        }
        nfmod += 1;
    }


    if(verbosity)
        print_changes(mode, old_mode, verbosity, file_name);

    return 0;
}


int run_xmod(char* in, char* file_name, int verbosity, int recursive, int argc, char* argv[]){
    struct stat st;
    if(stat(file_name, &st) != 0) {
        perror("stat");
        return 1;
    }

    __mode_t arg_info = st.st_mode;
    strcpy(curr_file, argv[argc - 1]);
    if (recursive) {
        if ((arg_info & __S_IFDIR) != 0) {
            if(recursive_xmod(in, file_name, verbosity, argc, argv)){
                return 1;
            }
            return 0;
        } 
        else {
            return 1;
        }
    } 
    else {
        nftot += 1;
        if(xmod(in, file_name, verbosity) != 0){
            perror("chmod");
            return 1;
        }
        return 0;
    }
}

int parse_perm_arg(char* arg){
    if(strlen(arg) < 3) return 1;

    enum state_machine {TARGET, OPERATOR, FIRST, SECOND, THIRD, DONE};
    enum state_machine sm = TARGET;
    for(int i = 0; i < strlen(arg); i++){
        if(sm == TARGET){
            if(arg[i] == 'a' || arg[i] == 'g' || arg[i] == 'u') {
                sm = OPERATOR;
                continue;
            }
            return 1;
        } else if (sm == OPERATOR){
            if(arg[i] == '+' || arg[i] == '-' || arg[i] == '='){
                sm = FIRST;
                continue;
            }
            return 1;
        } else if(sm == FIRST){
            if(arg[i] == 'r'){
                sm = SECOND;
                continue;
            } else if (arg[i] == 'w'){
                sm = THIRD;
                continue;
            } else if (arg[i] == 'x'){
                sm = DONE;
                continue;
            }
            return 1;
        } else if (sm == SECOND){
            if(arg[i] == 'w'){
                sm = THIRD;
                continue;
            } else if(arg[i] == 'x'){
                sm = DONE;
                continue;
            }
            return 1;
        } else if (sm == THIRD){
            if(arg[i] == 'x'){
                sm = DONE;
                continue;
            }
            return 1;
        } else if(sm == DONE){
            return 1;
        }
    }
    return 0;
}

int parse_perm_arg_octal(char* arg){
    if(strlen(arg) != 4) return 1;
    if(arg[0] != '0') return 1;
    for(int i = 1; i < 4; i++) if(atoi(&arg[i]) > 7 || atoi(&arg[i]) < 0) return 1;
    return 0;
}

int parse_argv(int argc, char* argv[]) {
    if (argc <= 2) {
        return 1;
    }

    if (access(argv[argc - 1], F_OK)) {
        perror("access");
        return 1;

    }

    int perms = argc - 2, start_index = 1;
    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-'){
            start_index++;
            perms--;
        }
        else
            break;
    }
    for (int i = start_index; perms > 0; i++, perms--) {
        if(parse_perm_arg(argv[i]) && parse_perm_arg_octal(argv[i])) return 1;
    }
    return 0;
}

/**
 * Gets the input on argv into a string
 *@param str destination of the input in argv
 */ 
void format_argv(int argc, char *argv[], char* str){
    strcpy(str, "");
    for (int i = 0; i < argc; i++){
        strcat(str, " ");
        strcat(str, argv[i]);
    }
}


int main(int argc, char* argv[], char* envp[]){
    char exit_code[2];
    strcpy(exit_code, "0");

    // verificar se há argumentos suficientes para correr o programa
    // Outdated, devido às opções já não funciona como deve
    if(parse_argv(argc, argv)){
        fprintf(stderr, "Invalid arguments.\n");
        strcpy(exit_code, "1");
    } else {
        char* str = (char *) malloc(100*sizeof(char));
        format_argv(argc, argv, str);
        
        if(set_handlers()){
            strcpy(exit_code, "1");
        } else {
            log_start();
            write_to_log(PROC_CREATE, str);

            int verbosity = 0, recursive = 0, index;
            if(get_options(&verbosity, &recursive, &index, argc, argv)){
                strcpy(exit_code, "1");
            } else {
                char *input = argv[index];
                char *in = (char *) malloc (18 * sizeof(char));
                char *file_name = argv[argc - 1];

                get_input(input, in, file_name, index, argc, argv);
                curr_file = file_name;

                if(run_xmod(in, file_name, verbosity, recursive, argc, argv) != 0){
                    strcpy(exit_code, "1");
                }
                free(in);
            }
        }
        free(str);
    }
    

    write_to_log(PROC_EXIT, exit_code);
    if(getpid() == FIRST_PROCESS_PID) wait(0);

    return atoi(exit_code);
}

