#ifndef PERMS_H_
#define PERMS_H_

#include <unistd.h>

/**
 * @brief Converts the given permissions to the respective mode bits.
 * While parse_perms does the formatting of the string, this function actually
 * converts the bits according to the specified mode changes.
 * 
 * @return New mode bits of the file, regarding a specific change (only processes one change at a time).
 */ 
__mode_t get_perms(unsigned int r, unsigned int w, unsigned int x, char op, char target, char* filename);

/**
 * @brief Converts the permissions given by the user to the respective mode bits,
 * and incorporates the changes into the files mode bits, preserving the necessary ones.
 * This function only parses the argument and adds the changes to the file mode.
 * 
 * @return New mode bits of the file.
 */ 
__mode_t parse_perms(char* perms, char* filename, int verbosity);

#endif  // PERMS_H_
