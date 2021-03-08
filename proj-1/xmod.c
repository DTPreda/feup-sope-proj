#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

__mode_t parse_perms(char* perms, char* filename, int verbosity);
__mode_t get_perms(unsigned int r, unsigned int w, unsigned int x, char op, char target, char* filename, int verbosity);
void chmod_dir(char* cmd, char* dir_name, int verbosity);
char * formatOctal(char *octal);


__mode_t parse_perms(char* perms, char* filename, int verbosity){
    size_t len = strlen(perms);
    char target = 'a';
    char mode;
    unsigned int read = 0, write = 0, execute = 0;
    for(int i = len - 1; perms[i] != '+' && perms[i] != '-' && perms[i] != '=' && i > 0; i--){
        if(perms[i] == 'r') read = 1;
        if(perms[i] == 'w') write = 1;
        if(perms[i] == 'x') execute = 1;
    }

    if(!read && !write && !execute){
	 fprintf(stderr, "Invalid input\n");
	 exit(-1);
    }

    switch (perms[0]){
        case 'u':
            target = 'u';
            mode = perms[1];
            break;
        case 'g':
            target = 'g';
            mode = perms[1];
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

    return get_perms(read, write, execute, mode, target, filename, verbosity);
}


__mode_t get_perms(unsigned int r, unsigned int w, unsigned int x, char op, char target, char* filename, int verbosity){
    __mode_t ret = 0;
    __mode_t old = 0; //to check if any change was made to the mode
    struct stat stb;
    if(stat(filename, &stb) != 0){	//get permissions
        perror("Stat");
    }
    ret = stb.st_mode;
    old = stb.st_mode;

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
    } else if (op == '='){
        // zeroes all target permissions before adding
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                ret &= ~(targets[j] << (((2 - i) + 3*j)));
            }
        }
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                ret |= modes[i]*targets[j] << ((2 - i) + 3*j);
            }
        }
    } else if (op == '-'){
        //ret = permissões do ficheiro
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                ret &= ~(modes[i]*targets[j] << (((2 - i) + 3*j)));
            }
        }
    }

    if (ret == old && verbosity == 1)
        printf("No changes were made to the file!\n");
    if (ret != old && verbosity)
        printf("File was changed from %o to %o\n", old, ret);

    return ret;
}

void chmod_dir(char* cmd, char* dir_name, int verbosity){
    //filename points to a dir
    char filename[100];
    DIR* d;
    struct dirent *dir;
    d = opendir(dir_name);
    if(d) {
        while((dir = readdir(d)) != NULL){
            if(dir->d_type == DT_REG) { //if it is a regular file
                strcpy(filename, "./");
                strcat(filename, dir_name); strcat(filename, "/"); // filename = dir_name/
                strcat(filename, dir->d_name);

                __mode_t mode = parse_perms(cmd, filename, verbosity);

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
                    return chmod_dir(cmd, filename, verbosity);
                } else {
                    wait(0);
                }
            }
        }

        __mode_t mode = parse_perms(cmd, dir_name, verbosity);
        if(chmod(dir_name, mode) != 0){
            perror("chmod");
            exit(1);
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
            strcat(result, " g+");
        }
        else if (i == 2){
            strcat(result, " o+");
        }
    }
    return result;
}

int main(int argc, char* argv[]){
    
    if(argc <= 2){
        fprintf(stdout, "Invalid number of arguments\n");
        exit(1);
    }

    int verbose = 0;
    int recursive = 0;
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
    }
    else{
        for (int i = index; i < argc - 1; i++){ //get all inputs u=rwx g=rx o=wx
            strcat(in, argv[i]);
            strcat(in, " ");
            //printf("String: %s\n", in);
        }
    }
    
    char * newInput= strtok(in, " ");   //split the string through the space
    // loop through the string to extract all other tokens
    while( newInput != NULL ) {
        if (recursive) {
            if ((arg_info & __S_IFDIR) != 0) {
                chmod_dir(newInput, file_name, verbose);
            }
            else {
                fprintf(stderr, "Invalid option, not a directory.\n");
                exit(-1);
            }
        }

        __mode_t mode = parse_perms(newInput, file_name, verbose);
        if(chmod(file_name, mode) != 0){
            perror("chmod");
            exit(-1);
        }

        newInput = strtok(NULL, " ");
    }

    return 0;
}
