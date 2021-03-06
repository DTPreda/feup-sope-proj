#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

__mode_t parse_perms(char* perms, char* filename);
__mode_t get_perms(unsigned int r, unsigned int w, unsigned int x, char op, char target, char* filename);

__mode_t parse_perms(char* perms, char* filename){
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
	 printf("Ganda murcao puseste isso mal ze\n");
	 exit(1);
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

    return get_perms(read, write, execute, mode, target, filename);
}


__mode_t get_perms(unsigned int r, unsigned int w, unsigned int x, char op, char target, char* filename){
    __mode_t ret = 0;
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
        struct stat stb;
        if(stat(filename, &stb) != 0){	//get permissions
            perror("Stat");
        }
        ret = stb.st_mode;
        //ret = permissões do ficheiro;
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                ret |= modes[i]*targets[j] << ((2 - i) + 3*j);
            }
        }
    } else if (op == '='){
        //ret está vazio; tudo a 0
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                ret |= modes[i]*targets[j] << ((2 - i) + 3*j);
            }
        }
    } else if (op == '-'){
        struct stat stb;
        if(stat(filename, &stb) != 0){
            perror("Stat");
        }
        ret = stb.st_mode;
        //ret = permissões do ficheiro
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                ret &= ~(modes[i]*targets[j] << (((2 - i) + 3*j)));
            }
        }
    }

    return ret;
}

void chmod_dir(char* cmd, char* dir_name){
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

                __mode_t mode = parse_perms(cmd, filename);

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
                    return chmod_dir(cmd, filename);
                } else {
                    wait(0);
                }
            }
        }

        __mode_t mode = parse_perms(cmd, dir_name);
        //printf("Im the directory: %s", argv[2]);
        if(chmod(dir_name, mode) != 0){
            perror("chmod");
            exit(1);
        }
    }
}

int main(int argc, char* argv[]){
    if(argc <= 2){
        fprintf(stdout, "Invalid number of arguments");
        exit(1);
    }

    struct stat st;
    if(stat(argv[2], &st) != 0) {
        perror("stat");
        exit(1);
    }

    __mode_t arg_info = st.st_mode;
    if((arg_info & __S_IFDIR) != 0){
        chmod_dir(argv[1], argv[2]);

    } else {
        //filename points to a file
        __mode_t mode = parse_perms(argv[1], argv[2]);
	//printf("Im the directory: %s", argv[2]);
        if(chmod(argv[2], mode) != 0){
            perror("chmod");
            exit(1);
        }
    }

    return 0;
}
