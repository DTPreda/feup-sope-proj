#ifndef _INOUTPUT_H_
#define _INOUTPUT_H_

/**
 * @brief Parses the arguments given by the user to check if there are no errors.
 * 
 * @param argc Number of arguments in argv.
 * 
 * @param argv Arguments given by the user at the start of the program.
 * 
 * @return 0 if input is valid, 1 otherwise.
 */ 
int parse_argv(int argc, char* argv[]);

/**
 * @brief Helper function of parse_argv to check if, given the mode in octal format,
 * said format is valid or not. 
 * 
 * @param arg File mode bits the user wishes to set, given in octal.
 * 
 * @return 0 if input is valid, 1 otherwise.
 */ 
int parse_perm_arg_octal(char* arg);

/**
 * @brief Helper function of parse_argv to check if, given the mode by the "standard"
 * way, said mode is valid or not.
 * 
 * @param arg File mode bits the user wishes to add, remove or set, given in the "standard" way.
 * 
 * @return 0 if input is valid, 1 otherwise.
 */ 
int parse_perm_arg(char* arg);

/**
 * @brief Formats the input of argv into a single string.
 * 
 * @param argc Number of arguments in argv.
 * 
 * @param argv Arguments given by the user at the start of the program.
 * 
 * @param str destination of the input in argv.
 */ 
void format_argv(int argc, char *argv[], char* str);

/**
 * @brief Converts octal input to format "u=--- g=--- o=---", where "-" can be 'r' 'w' or 'x'.
 * 
 * @param octal Mode bits given by the user, in octal mode.
 * 
 * @param in String that will be receiving the input in the "standard" format.
 */ 
void format_octal(char *octal, char* in);

/**
 * @brief Records the options given by the user with the flags available,
 * and updates the respective variables. Also sets index to the first
 * argument after the options, for ease of parsing.
 * 
 * @param verbose Verbosity flag. Set to 0 for no verbosity, 1 to print
 *  everything and 2 for only printing changes made. 
 * 
 * @param recursive Recursive flag. Set to 1 for recursive mode. 
 * Whem recursive mode is set, xmod will only run on directories.
 * Likewise, if not set, it will only run on a single file
 * (wich can be a directory).
 * 
 * @param index Index of the first argument after the options.
 * Used for ease of parsing.
 * 
 * @param argc Number of arguments in argv.
 * 
 * @param argv Arguments given by the user at the start of the program.
 * 
 * @return 0 if successfull, 1 otherwise
 */ 
int get_options(int* verbose, int* recursive, int* index, int argc, char* argv[]);

/**
 * @brief Formats the file mode bits given, either in octal or the "standard"
 * way, into a single string, in the "standard" format of u=--- g=--- o=---
 * where --- is a combination of r, w and x.
 * 
 * @param input Input given by the user.
 * 
 * @param in String to be formatted in the final form.
 * 
 * @param index Index, in the argv array, of the first argument that is
 * not an option (like -v, or -cR).
 * 
 * @param argc Number of arguments in argv.
 * 
 * @param argv Arguments given by the user at the start of the program.
 */ 
void get_input(char* input, char* in, int index, int argc, char* argv[]);

/**
 * @brief Writes to the log file and, if verbosity is set, to the standard output,
 * information about the changes of files and the old and new modes of said file.
 * 
 * @param new_mode Updated mode bits of the file.
 * 
 * @param old_mode Original mode bits of the file.
 * 
 * @param verbosity Flag to check if information must be written to the
 * standard output, and what changes should be written, if any.
 * 
 * @param file_name Name of the file affected by the change.
 */ 
void print_changes(__mode_t new_mode, __mode_t old_mode, int verbosity, char* file_name);

/**
 * @brief Converts the mode of the permissions from bits to
 * the format rwxrwxrwx.
 *
 * @param mode Mode to be converted to string.
 * 
 * @param buf String where the text will be written.
 */ 
void str_mode(__mode_t mode, char * buf);

/**
 * @brief Determines where the executable is stored, in order to run
 * the program without having to worry about placement of the binary.
 * 
 * @param argv Arguments given by the user at the start of the program.
 * 
 * @return 0 if search is successfull, 1 otherwise
 */ 
int determine_executable_path(char* argv);

#endif  // _INOUTPUT_H_
