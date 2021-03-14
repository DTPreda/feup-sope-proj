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

struct timespec start, end;

__mode_t parse_perms(char* perms, char* filename, int verbosity);
__mode_t get_perms(unsigned int r, unsigned int w, unsigned int x, char op, char target, char* filename);
void chmod_dir(char* cmd, char* dir_name, int verbosity, int argc, char* argv[], char* envp[]);
char * formatOctal(char *octal);
void strmode(__mode_t mode, char * buf);
double getRunningTime();

/**
 * Gets the time that the process runned until the moment this function is called
 @return double with the time 
*/
double getRunningTime(){
    sleep(1);
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    double delta_ms = (double)((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000)/1000;
    fprintf(stdout, "Elapsed time(ms): %f", delta_ms);
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

    if (ret != old && verbosity)
        printf("mode of '%s' changed from 0%o (%s) to 0%o (%s)\n", filename, old % 512, oldMode, ret % 512, newMode);

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

        __mode_t mode = parse_perms(cmd, dir_name, verbosity);
        if(chmod(dir_name, mode) != 0){
            perror("chmod");
            exit(1);
        }

        while((dir = readdir(d)) != NULL){

            strcpy(copy, cmd);
            if(dir->d_type == DT_REG || dir->d_type == DT_LNK) { //if it is a regular file
                strcpy(filename, "");
                strcat(filename, dir_name); strcat(filename, "/"); // filename = dir_name/
                strcat(filename, dir->d_name);

                __mode_t mode = parse_perms(copy, filename, verbosity);
                if(chmod(filename, mode) != 0){
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
                    strcat(cmd, filename);

                    argv[argc - 1] = filename;
                    if (execve("xmod", argv, envp) == -1)
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


int main(int argc, char* argv[], char* envp[]){
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);


    if(argc <= 2){
        fprintf(stdout, "Invalid number of arguments\n");
        exit(1);
    }
    
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("signal");
        exit(-1);
    }

    int verbose = 0;
    int recursive = 0;
    int log = 0;
    int option;
    int index;
    while ((option = getopt(argc, argv, "vcR")) != -1)
    {
        switch (option) {
            case 'v':
                verbose = 1;
                break;
            case 'c':
                verbose = 2;
                break;
            case 'R':
                recursive = 1;
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

    char *logFile;
    if ((logFile = getenv("LOG_FILENAME")) != NULL) {
        log = 1;
    }

    if (log)
    {
        FILE* fd = fopen(logFile, )
        
    }
    

    index = optind;

    char *input = argv[index];
    
    char *in = (char *) malloc (18 * sizeof(char));

    char *file_name = argv[argc - 1];

    struct stat st;
    if(stat(file_name, &st) != 0) {
        perror("stat");
        exit(-1);
    }

    __mode_t arg_info = st.st_mode;

    if (input[0] == '0'){  //get input 
        in = formatOctal(input);
    } else {
        for (int i = index; i < argc - 1; i++){ //get all inputs u=rwx g=rx o=wx
            strcat(in, argv[i]);
            strcat(in, " ");
            //printf("String: %s\n", in);
        }
    }
    
    if (recursive) {
        if ((arg_info & __S_IFDIR) != 0) {
            chmod_dir(in, file_name, verbose, argc, argv, envp);
            return 0;
        } else {
            fprintf(stderr, "Invalid option, not a directory.\n");
            exit(-1);
        }
    }

    __mode_t mode = parse_perms(in, file_name, verbose);
    if(chmod(file_name, mode) != 0){
        perror("chmod");
        exit(-1);
    }
    

    return 0;
}

