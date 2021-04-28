# A Client-Server Application to manage requests

---

This is a project to create a multithreaded client-server application where several requests with different weights are sent to be executed. Such requests are executed by an external library, who only communicates with the server. Initially, a development of a Client will be made, while the Server will be constructed in more advanced parts, after a functional Client is achieved.

## Compilation

A Makefile has been provided, as well as an object file of the Server, to be executed. In order to compile the Client, the user only needs to call the Makefile with ```make``` and the code will be compiled using the correct flags.

## Executing the client

As stated in the project description, the client must be called with:

```./c -t nsecs fifoname``` 
    
Where:

- nsecs - approximate number of seconds that the Client should work for
- fifoname - absolute or relative name of the public communication channel between the Client and the Server.

## Executing the Server

Once again, in the project description it is stated that the Server must be called with:

```./s -t nsecs [-l buffsize] fifoname```

Where:

- nsecs - approximate number of seconds that the Server should work for
- buffsize - optional argument to set the size of the buffer used to store the execution results
- fifoname - absolute or relative name of the public communication channel between the Client and the Server.

## Communication between the Client and the Server

The application **must comply** with the following communication protocols:

- Every request must have a response.
- The points of communication are:
    - A public FIFO, through which requests are sent.
    - A private FIFO per thread created, trough which the Server response will be sent, whose name's will have the structure ```/tmp/pid.tid```, where:
        - pid - Client's process identifier
        - tid - thread identifier
- Messages are sent via the same struct, which is given as ```struct msg{int i; int t; pid_t pid; pthread_t tid; int res};```, where:
    - i - request ID
    - t - task weight
    - pid - process identifier
    - tid - thread identifier
    - res - if coming from Client, always -1, if coming from Server, either the result given by the task execution or -1 if the service has ended

## Code structure

Code separation was heavily taken into account, as to follow the Single Responsibility Principle. Each module serves a distinct purpose, and here, it was decided to keep them in the same file due to the reduced size of each one, as well as the similarities between the objectives of each function. As such, the code can be broken down into 3 major components, which are:

- main - The main function responsible for the creation and termination of each thread.

- Requests - The set of functions responsible for creating the request, sending it and waiting for its response. The main functions belonging to this area are the ```request, request_setup, make_request and get_result ```

- Utilities - The set of utilitarian functions responsible for all the other tasks, such as registering the operations in stdout, verifying if the arguments are valid or finding the time passed since the start of execution.

The flow of the program is then as follows:

- First, the main function checks for the correct arguments and argument list. If anything is invalid, the program terminates here.

- The program starts creating the threads at a steady rate, each of them calling the ```request``` function.

- Each thread then creates a request and sends it to the server through the public FIFO created.

- After the server issues a response through the private FIFO belonging to each thread, the response is registerd and cleanup is done, removing the pipe and terminating the thread's job (but not eliminating the thread yet)

- When time is up, or the server shuts down, the remaining cleanup is done, freeing all memory left and, if open, the public FIFO is closed. Also, every thread is terminated at the end. At last, the program terminates.

## Implementation details

TODO

### Self evaluation

Since Tiago was useless in this project (and life in general), and completely carried by his friends, he will be very sad in a corner for the rest of the week, drinking to forget his many many problems.