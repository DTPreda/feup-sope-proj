#ifndef PERMS_H_
#define PERMS_H_

#include <unistd.h>

__mode_t get_perms(unsigned int r, unsigned int w, unsigned int x, char op, char target, char* filename);
__mode_t parse_perms(char* perms, char* filename, int verbosity);

#endif  // PERMS_H_
