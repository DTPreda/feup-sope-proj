#ifndef _INOUTPUT_H_
#define _INOUTPUT_H_

int parse_argv(int argc, char* argv[]);
int parse_perm_arg_octal(char* arg);
void format_argv(int argc, char *argv[], char* str);
int parse_perm_arg(char* arg);
char * format_octal(char *octal);
int get_options(int* verbose, int* recursive, int* index, int argc, char* argv[]);
void get_input(char* input, char* in, char* file_name, int index, int argc, char* argv[]);
void print_changes(__mode_t new_mode, __mode_t old_mode, int verbosity, char* file_name);
void str_mode(__mode_t mode, char * buf);

#endif  // _INOUTPUT_H_
