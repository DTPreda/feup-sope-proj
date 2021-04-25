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

TODO

## Implementation details

TODO

### Self evaluation

Since Tiago was useless in this project (and life in general), and completely carried by his friends, he will be very sad in a corner for the rest of the week, drinking to forget his many many problems.