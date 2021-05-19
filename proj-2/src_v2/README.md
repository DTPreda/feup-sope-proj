# A Client-Server Application to manage requests

---

This is a project to create a multithreaded client-server application where several requests with different weights are sent to be executed. Such requests are executed by an external library, who only communicates with the server. Initially, a development of a Client will be made, while the Server will be constructed in more advanced parts, after a functional Client is achieved.

## Compilation

A Makefile has been provided, as well as an object file of the Server, to be executed. In order to compile the Client, the user only needs to call the Makefile with ```make``` and the code will be compiled using the correct flags.

To successfully compile both the client and the server, the Makfile has been updated. This means that the user still only needs to enter the ```make``` command
to successfully compile with the correct flags.

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

## Client code structure

Code separation was heavily taken into account, as to follow the Single Responsibility Principle. Each module serves a distinct purpose, and here, it was decided
to keep them in the same file due to the reduced size of each one, as well as the similarities between the objectives of each function. As such, the Client's
code can be broken down into 3 major components, which are:

- main - The main function responsible for the creation of each thread.

- Requests - The set of functions responsible for creating the request, sending it and waiting for its response. The main functions belonging to this area are the ```request, request_setup, make_request and get_result```

- Utilities - The set of utilitarian functions responsible for all the other tasks, such as registering the operations in stdout, verifying if the arguments are valid or finding the time passed since the start of execution.

The flow of the program is then as follows:

- First, the main function checks for the correct arguments and argument list. If anything is invalid, the program terminates here.

- The program starts creating the threads at a steady rate, each of them calling the ```request``` function.

- Each thread then creates a request and sends it to the server through the public FIFO created.

- After the server issues a response through the private FIFO belonging to each thread, the response is registerd and cleanup is done, removing the pipe and terminating the thread's job

- When time is up, or the server shuts down, the remaining cleanup is done, freeing all memory left and, if open, the public FIFO is closed. Also, every thread is terminated at the end. At last, the program terminates.

## Client implementation details

The client program can be divide in two parts: the main client thread and the requesting client threads. The main client thread creates requesting client threads at a random frequency and the requesting client threads are responsible are responsible of client-server communication. In order to do that, each requesting client threads creates a private fifo in the form of "/tmp/pid.tid" and a message, which tells the Server what the Client requests. To create such message, a random number between 1 and 9 is created in order to specify the task priority. Each request has it specific id, which is obtained through a global variable, incremented by 1 upon each request.

To make a request, Client sends a message through the public pipe created by the Server, through the write() system call. Alongisde write(), the select() function was used. We considered the usage of this function to control the file descriptors of the public pipe and to control the execution time of the requesting threads, given that they should not make requests if their running time passes the time factor introduced on the arguments of the program. To get the response of each task, the same approach was used, but instead of writing to the public fifo, we read the private fifo created in the beginning of each requesting thread.

After the message is received or the running time reaches to limit, the unlink() system call is used on the private pipe.

To avoid limitations, the requesting threads are created in detached mode. By not having to join later, we do not need to keep each thread's ID, as they will naturally die after their task is completed. Thus, we do not have to hardcode a fixed number of threads, and we can recycle threads.
The main thread is exited upon finishing thread creation. The program only ends when the last active thread dies, after which the public fifo file descriptor is closed, so the server can end correctly as well.

## Server code structure

Once again, all code was kept in the same file as all functions serve a similar purpose, and the file itself maintains a single responsibility (managing the
server). With that said, each function has a distinct purpose, and modulatiry was taken heavily taken into account. The server code can be divided into a similar
structure to that of the client. As such, there are these main sections:

- main - A main function responsible for providing the overall flow of the program.

- Request handling - Set of functions responsible for receiving requests, sending them to the library and storing the results. They are also the section that
effectivelly communicates with the client.

- Utility - The remaining utility functions, which serve as helpers to the Request handling, in tasks such as registering results to standard output, see if the
time is up, inserting and retreiving items from the queue, etc.

## Server implementation details

! Mention queue structure built from scratch
! Mention file structure for cpplint

### Self evaluation

The group believes work was distributed evenly between all elements, and so everyone agreed that the participation was of a third for each element, or 33.(3)%
