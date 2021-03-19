#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "./xmod.h"
#include "./sig_handling.h"
#include "./log.h"
#include "./inoutput.h"

char executable_path[FILENAME_MAX];

void format_argv(int argc, char *argv[], char* str) {
    strcpy(str, "");
    for (int i = 0; i < argc; i++) {
        strcat(str, " ");
        strcat(str, argv[i]);
    }
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
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            start_index++;
            perms--;
        } else {
            break;
        }
    }
    for (int i = start_index; perms > 0; i++, perms--) {
        if (parse_perm_arg(argv[i]) && parse_perm_arg_octal(argv[i])) return 1;
    }
    return 0;
}


int parse_perm_arg_octal(char* arg) {
    if (strlen(arg) != 4) return 1;
    if (arg[0] != '0') return 1;
    if (atoi(arg) < 0 || atoi(arg) > 777) return 1;
    return 0;
}


int parse_perm_arg(char* arg) {
    if (strlen(arg) < 3) return 1;

    enum state_machine {TARGET, OPERATOR, FIRST, SECOND, THIRD, DONE};
    enum state_machine sm = TARGET;
    for (int i = 0; i < strlen(arg); i++) {
        if (sm == TARGET) {
            if (arg[i] == 'a' || arg[i] == 'g' || arg[i] == 'u' || arg[i] == 'o') {
                sm = OPERATOR;
                continue;
            }
            return 1;
        } else if (sm == OPERATOR) {
            if (arg[i] == '+' || arg[i] == '-' || arg[i] == '=') {
                sm = FIRST;
                continue;
            }
            return 1;
        } else if (sm == FIRST) {
            if (arg[i] == 'r') {
                sm = SECOND;
                continue;
            } else if (arg[i] == 'w') {
                sm = THIRD;
                continue;
            } else if (arg[i] == 'x') {
                sm = DONE;
                continue;
            }
            return 1;
        } else if (sm == SECOND) {
            if (arg[i] == 'w') {
                sm = THIRD;
                continue;
            } else if (arg[i] == 'x') {
                sm = DONE;
                continue;
            }
            return 1;
        } else if (sm == THIRD) {
            if (arg[i] == 'x') {
                sm = DONE;
                continue;
            }
            return 1;
        } else if (sm == DONE) {
            return 1;
        }
    }
    return 0;
}


void format_octal(char *octal, char* in) {
    strcpy(in, "u");
    for (int i = 1; i < strlen(octal); i++) {
        switch (octal[i]) {
        case '7':
            strcat(in, "=rwx");
            break;
        case '6':
            strcat(in, "=rw");
            break;
        case '5':
            strcat(in, "=rx");
            break;
        case '4':
            strcat(in, "=r");
            break;
        case '3':
            strcat(in, "=wx");
            break;
        case '2':
            strcat(in, "=w");
            break;
        case '1':
            strcat(in, "=x");
            break;
        case '0':
            strcat(in, "-rwx");
            break;
        default:
            break;
        }

        if (i == 1) {
            strcat(in, " g");
        } else if (i == 2) {
            strcat(in, " o");
        }
    }
}


int get_options(int* verbose, int* recursive, int* index, int argc, char* argv[]) {
    int option;
    while ((option = getopt(argc, argv, "vcR")) != -1) {
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


void get_input(char* input, char* in, int index, int argc, char* argv[]) {
    if (input[0] == '0') {  // get input
        format_octal(input, in);
    } else {
        strcpy(in, "");
        for (int i = index; i < argc - 1; i++) {  // get input in the form
            strcat(in, argv[i]);                  // u=rwx g=rx o=wx
            strcat(in, " ");
        }
    }
}


void print_changes(__mode_t new_mode, __mode_t old_mode, int verbosity, char* file_name) {
    if (verbosity == 3) {
        printf("neither symbolic link '%s' nor referent has been changed\n", file_name);
        return;
    }

    char old_mode_str[15];
    char new_mode_str[15];
    str_mode(old_mode, old_mode_str);
    str_mode(new_mode, new_mode_str);
    if (new_mode == old_mode && verbosity == 1) {
        printf("mode of '%s' retained as 0%o (%s)\n", file_name, new_mode % 512, old_mode_str);
    } else if (new_mode != old_mode && verbosity) {
        char str[100];
        snprintf(str, sizeof(str), "%s : %o : %o", file_name, old_mode % 512, new_mode % 512);
        write_to_log(FILE_MODF, str);
        printf("mode of '%s' changed from 0%o (%s) to 0%o (%s)\n", file_name, old_mode % 512, old_mode_str, new_mode % 512, new_mode_str);
    }
}


void str_mode(__mode_t mode, char * buf) {
  const char chars[] = "rwxrwxrwx";
  for (size_t i = 0; i < 9; i++) {
    buf[i] = (mode & (1 << (8-i))) ? chars[i] : '-';
  }
  buf[9] = '\0';
}

int determine_executable_path(char* argv) {
    if (argv[0] == '/') {
        strcpy(executable_path, argv);
        return 0;
    }

    for (int i = 0; i < strlen(argv); i++) {
        if (argv[i] == '/') {
            char buff[FILENAME_MAX];
            getcwd(buff, FILENAME_MAX);
            strcpy(executable_path, buff);
            strcat(executable_path, "/");
            strcat(executable_path, argv);
            return 0;
        }
    }

    char path[FILENAME_MAX];
    strcpy(path, getenv("PATH"));

    char* input = strtok(path, ":");
    char tmp[FILENAME_MAX];
    for ( ; input != NULL; ) {
        strcpy(tmp, input); strcat(tmp, "/xmod");
        if (access(tmp, F_OK) == 0) {
            strcpy(executable_path, tmp);
            return 0;
        }
        input = strtok(NULL, ":");
    }

    return 1;
}
