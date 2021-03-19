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

## Code Structure

The code is divided in 5 major groups:
- xmod
    - Main block of code, responsible for tying together all the other groups. The function main resides in this group, and it prepares all the input received in order to correctly run xmod. In case any misuse is detected, the program never leaves actually gets to xmod itself.
    - It contais xmod itself, which consists of getting the required permissions for the file, and setting them, through chmod system call. It also contains its recursive version, along some other functions related to the execution of xmod.
- input and output
    - Contains all code related to parsing and verifying correctness of the arguments. It communicates to main if any error or misuse is detected and correctly formats all the input so the following code can run properly.
- perms
    - Contains all code related to file permissions, from parsing the commands received to actual permissions that can be used by the other functions. Also contains the blocks of code responsible of building the permissions (or maintaining them) the user required, to later be used in xmod. 
- log
    - Contains all code related to the log. It's throught here that the start time of the first process will be saved, and, in case the environment variable LOG_FILENAME is defined, that the information is written to log. In case it isn't, most of the code will not be used.
- sig_handling
    - Contains all code related to signal handling. Only contains two function: sig_handler, which writes received 1 to 31 signals to LOG_FILENAME (if defined), and has specific actions defined for SIGINT and SIGUSR1, to meet project requirement, and set_handlers, which sets sig_handler as the handler for signals 1 to 31.

The overall flow of the code is as follows:
- All arguments are parsed and their correctness are verified. If the file indicated does not exist or some argument is incorrect, the program ends here;
- Arguments are then formatted into a single string;
- Signal handlers are set. If it fails, the program ends here;
- Log related variables and log itself are initialized. If it fails, the program ends here;
- Process creation is indicated in log.
- Flag options are determined. If it fails, the program ends here;
- Arguments are now parsed to extract the information needed to later be used in xmod itself. As its correctness was already verified, this won't fail nor end the program.
- Control is then passed to run_xmod, which will either run a single instance of xmod, or its recursive variant. It may end the program before changing any permission, in the case some requirements aren't met.
Flow of control will occasionaly pass through log and signals related functions, whenever it is needed.

## Implementation Details

Xmod was written in a modular fashion, where every function serves a distinct purpose. Also, exit points were minimized to provide better flow of control regarding both signal handling and log writing (minimizes function calls). Regarding signal handling, at any point the user may choose to interrupt the program (by giving a SIGINT, usually by writing Ctrl+C on the console). This causes a halt on the processes, by using a SIGSTOP on every child process, and the user may choose to either continue execution, or stop the program entirely. If the user wishes to continue, a SIGCONT is sent to enable the processes to resume their execution, but when the user wants to stop the, after the SIGCONT a SIGUSR1 is sent. This notifies the child processes that they have to terminate, and the parent process waits for all of them to finish before stopping.

In order to control the program execution time, we used an environment variable called START_TIME that stores the time since the program has started, in order to register the events in order at the log file.

Finally, another feature of xmod is that it allows to change more than one target at a time. This means that, if needed, the user can change the mode bits to himself, his group, or the other elements, in a single command. All that is needed is for the different permissions to be separated by spaces, to be recognized. The user doesn't need to worry about using invalid input. Xmod checks if the input is valid before executing any actions, potentially damaging the file.


## Self Evaluation.

The group believes work was distributed evenly between all members, and so everyone agreed that the participation was of a third for each element, or 33.(3)%