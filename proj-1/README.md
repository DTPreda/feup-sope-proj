# Xmod: A utility to change file mode permissions

Xmod is a program developed as a subset of chmod, the official GNU utility to change the permissions of a file or set of files, in regards to the current user, his group and other users.

## Compilation

For the code to become an executable, a Makefile is provided. As such, the user needs only to call make on the program with ```make xmod``` for the compiler to be called with the correct flags.

## Executing the program

Executing the program is achieved by following the order of arguments detailed in the project specification, that is```./xmod [OPTIONS] (MODE | OCTAL-MODE) (FILE | DIR)```.

OPTIONS - One of 3 is possible, -v, -c and -R. -v is equivalent to the verbose mode of chmod, and presents information in the same format. -c is similar to -v, but only presents information when a change is made. -R stands for Recursive, and with this option enabled every file in the directory, including the directory itself, will be changed to the mode specified in MODE. When -R is called, the user must call a directory or otherwise the program will fail.

MODE/OCTAL-MODE - New mode of the file/directory, written according to the project specification. For the normal mode, it is possible for the user to change the 3 permissions separately (user, group and others), or, if easier, the user can opt for OCTAL-MODE, where the first number must be a zero to be recognized, followed by the octal representation of the mode bits the user wishes to set. For example, if we want everyone to be able to read (r), write (w) and execute (x) the file, one could simpy do ```./xmod 0777 file/dir```.

FILE/DIR - Name of the file or directory we wish to change. Xmod handles files in the same way that chmod would, and that includes symbolic links, where instead of the actual file being changed, only the file it points to is affected. When a directory is specified with the recursive option enabled, every file within the folder will be changed (including files within other folders and excluding the files pointed by the symbolic links), in the same way that chmod works, but when no recursive flag is given, only the directory will be changed.

In order to store the results of processes created/exited and signals sent/handled, the user must create an environment variable whose value is the file where information will be stored.
For that the user can set the environment variable writing the follwing on the command line: 
```export LOG_FILENAME=<name_of_file_to_store>```

## Implementation Details

Xmod was written in a modular fashion, where every function serves a distinct purpose. Also, exit points were minimized to provide better flow of control regarding both signal handling and log writing (minimizes function calls). Regarding signal handling, at any point the user may choose to interrupt the program (by giving a SIGINT, or by writing Ctrl+C on the console). This causes a halt on the processes, by using a SIGSTOP on every child process, and the user may choose to either continue execution, or stop the program entirely. If the user wishes to continue, a SIGCONT is sent to enable the processes to resume their execution, but when the user wants to stop the, after the SIGCONT a SIGUSR1 is sent. This signals notifies the child processes that they have to terminate, and the parent process waits for all of them to finish before stopping.
In order to control the program execution time of the program, we used an environment variable called START_TIME that stores the time when program run.