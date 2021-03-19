#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>


__mode_t get_perms(unsigned int r, unsigned int w, unsigned int x, char op, char target, char* filename) {
    __mode_t ret = 0;

    unsigned int modes[3] = {r, w, x};
    unsigned int targets[3] = {0, 0, 0};

    if (target == 'a') {
        targets[0] = 1;
        targets[1] = 1;
        targets[2] = 1;
    } else {
        if (target == 'u') targets[2] = 1;
        if (target == 'g') targets[1] = 1;
        if (target == 'o') targets[0] = 1;
    }
    if (op == '+') {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                ret |= modes[i]*targets[j] << ((2 - i) + 3*j);
            }
        }
    } else if (op == '-') {
        struct stat stb;
        if (stat(filename, &stb) != 0) {  // get permissions
            perror("Stat");
            return __UINT32_MAX__;
        }
        ret = stb.st_mode;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                ret &= ~(modes[i]*targets[j] << (((2 - i) + 3*j)));
            }
        }
    }
    return ret;
}


__mode_t parse_perms(char* perms, char* filename, int verbosity) {
    __mode_t ret = 0;
    struct stat stb;
    if (stat(filename, &stb) != 0) {  // get permissions
        perror("Stat");
        return __UINT32_MAX__;
    }
    ret = stb.st_mode;
    char* copy = (char *) malloc(strlen(perms) * sizeof(char) + 1) ;
    strcpy(copy, perms);
    char* input = strtok(copy, " ");
    for ( ; input != NULL ; ) {
        size_t len = strlen(input);
        char target = 'a';
        char mode;
        unsigned int read = 0, write = 0, execute = 0;
        for (int i = len - 1; input[i] != '+' && input[i] != '-' && input[i] != '=' && i > 0; i--) {
            if (input[i] == 'r') read = 1;
            if (input[i] == 'w') write = 1;
            if (input[i] == 'x') execute = 1;
        }

        if (!read && !write && !execute) {
            fprintf(stderr, "Invalid input\n");
            return __UINT32_MAX__;
        }

        switch (input[0]) {
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
        if (mode == '+') {
            temp = get_perms(read, write, execute, mode, target, filename);
            if (temp == __UINT32_MAX__) return __UINT32_MAX__;
            ret |= temp;
        } else if (mode == '=') {
            temp = get_perms(1, 1, 1, '+', target, filename);
            if (temp == __UINT32_MAX__) return __UINT32_MAX__;

            sec_temp = get_perms(read, write, execute, '+', target, filename);
            if (sec_temp == __UINT32_MAX__) return __UINT32_MAX__;

            ret &= ~(temp);
            ret |= sec_temp;
        } else {
            temp = get_perms(read, write, execute, mode, target, filename);
            if (temp == __UINT32_MAX__) return __UINT32_MAX__;

            ret &= temp;
        }

        input = strtok(NULL, " ");
    }
    free(copy);
    return ret;
}


