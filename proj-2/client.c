#include "client.h"

char* public_pipe;

int parse_args(int argc, char* argv[]){
    if (argc != 4) return 1;
    if (strncmp(argv[1], "-t", 2) != 0) return 1;
    if (argv[2] <= 0) return 1;
    if (mkfifo(argv[3], 0666) < 0) return 1;
    return 0;
}

int main (int argc, char* argv[]) {

    if(parse_args(argc, argv) != 0) {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }

    public_pipe = argv[3];

    return 0;
}