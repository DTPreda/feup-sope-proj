#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include "./log.h"
#include "./sig_handling.h"
#include "./inoutput.h"
#include "./xmod.h"
#include "./perms.h"

#define GetCurrentDir getcwd

extern struct timespec start_time, end_time;
extern long int time_start, time_end;
extern unsigned int nftot;
extern unsigned int nfmod;
extern char* curr_file;  // currently FILE/DIR passed to argv
extern char executable_path[FILENAME_MAX];

void concatenate_dir_file(char* dir, char* file_name, char* ret) {
    strcpy(ret, "");
    strcat(ret, dir);
    strcat(ret, "/");  // filename = dir_name/
    strcat(ret, file_name);
}

int recursive_xmod(char* cmd, char* dir_name, int verbosity, int argc, char *argv[]) {
    sleep(1);
    char copy[100];
    char file_name[FILENAME_MAX];
    DIR* d;
    struct dirent *dir;
    d = opendir(dir_name);
    
    if (d) {
        if (xmod(cmd, dir_name, verbosity) != 0) {
            perror("chmod");
            return 1;
        }

        nftot += 1;  // found dir called by argument

        while ((dir = readdir(d)) != NULL) {
            strcpy(copy, cmd);
            if (dir->d_type == DT_REG) {  // if it is a regular file
                nftot += 1;  // found a file inside the directory
                concatenate_dir_file(dir_name, dir->d_name, file_name);

                if (xmod(cmd, file_name, verbosity) != 0) {
                    perror("chmod");
                    return 1;
                }

            } else if (dir->d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                int pid;
                if ((pid = fork()) < 0) {
                    perror("fork");
                    return 1;
                }
                if (pid == 0) {
                    concatenate_dir_file(dir_name, dir->d_name, file_name);

                    argv[argc - 1] = file_name;
                    //printf("%s\n", executable_path);
                    if (execv(executable_path, argv) == -1) {
                        argv[argc - 1] = dir_name;
                        perror("execv");
                        return 1;
                    }
                    argv[argc - 1] = dir_name;
                } else {
                    wait(0);
                }
            } else if (dir->d_type == DT_LNK) {
                concatenate_dir_file(dir_name, dir->d_name, file_name);
                print_changes(0, 0, 3 & (3 * (verbosity == 1)), file_name);
            }
        }
        return 0;
    }
    return 1;
}



int xmod(char* in, char* file_name, int verbosity) {
    __mode_t old_mode = 0;
    struct stat stb;
    if (stat(file_name, &stb) != 0) {  // get permissions
        perror("Stat");
        return 1;
    }
    old_mode = stb.st_mode;

    __mode_t mode = 0;
    if ((mode = parse_perms(in, file_name, verbosity)) == __UINT32_MAX__)
         return 1;
    if (mode != old_mode) {
        if (chmod(file_name, mode) != 0) {
            perror("chmod");
            return 1;
        }
        nfmod += 1;
    }


    if (verbosity)
        print_changes(mode, old_mode, verbosity, file_name);

    return 0;
}


int run_xmod(char* in, char* file_name, int verbosity, int recursive, int argc, char* argv[]) {
    struct stat st;
    if (stat(file_name, &st) != 0) {
        perror("stat");
        return 1;
    }
    __mode_t arg_info = st.st_mode;
    if (recursive) {
        if(determine_executable_path(argv[0]) != 0){
            fprintf(stderr, "Could not find executable path, impossible to run recursively.\n");
            return 1;
        }
        if ((arg_info & __S_IFDIR) != 0) {
            if (recursive_xmod(in, file_name, verbosity, argc, argv)) {
                return 1;
            }
            return 0;
        } else {
            fprintf(stderr, "This file is not a directory\n");
            return 1;
        }
    } else {
        nftot += 1;
        if (xmod(in, file_name, verbosity) != 0) {
            perror("chmod");
            return 1;
        }
        return 0;
    }
}



int main(int argc, char* argv[], char* envp[]) {
    char exit_code[2];
    strcpy(exit_code, "0");

    if (parse_argv(argc, argv)) {
        fprintf(stderr, "Invalid arguments.\n");
        strcpy(exit_code, "1");
    } else {
        char str[FILENAME_MAX];
        format_argv(argc, argv, str);
        if (set_handlers()) {
            strcpy(exit_code, "1");
        } else {
            log_start();
            write_to_log(PROC_CREATE, str);
            int verbosity = 0, recursive = 0, index;
            if (get_options(&verbosity, &recursive, &index, argc, argv)) {
                strcpy(exit_code, "1");
            } else {
                char *input = argv[index];
                char in[18];
                char *file_name = argv[argc - 1];
                get_input(input, in, file_name, index, argc, argv);
                curr_file = file_name;
                if (run_xmod(in, file_name, verbosity, recursive, argc, argv) != 0) {
                    strcpy(exit_code, "1");
                }
            }
        }
    }


    write_to_log(PROC_EXIT, exit_code);
    if (getpid() == FIRST_PROCESS_PID) wait(0);

    return atoi(exit_code);
}