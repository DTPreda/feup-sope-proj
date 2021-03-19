#ifndef PERMS_H_
#define PERMS_H_

#include <unistd.h>

/**
 * @brief Converts the given permissions to the respective mode bits.
 * While parse_perms does the formatting of the string, this function actually
 * converts the bits according to the specified mode changes.
 * 
 * @param r Read Permisson. 0 if not enabled, 1 otherwise
 * 
 * @param w Write Permisson. 0 if not enabled, 1 otherwise
 * 
 * @param x Execute Permisson. 0 if not enabled, 1 otherwise
 * 
 * @param op One of +, - , =. For adding (+), subtracting (-) or defining (=) the permissions.
 * 
 * @param target One of u, g, o. Changes will affect either the user (u), group (g) or other users (o).
 * 
 * @param filename Name of the file whose permissions we wish to change.
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
