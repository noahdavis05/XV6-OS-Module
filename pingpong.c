#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    // declare an array of size 2 to hold 2 file descriptors for a pipe.
    int pipefd[2];
    // declare a character array to store the message sent through the pipe
    //char buffer[10];
    // declare an integer vqriable which stores the process id returned by fork system call
    int pid;

    // create a pipe
    if (pipe(pipefd) < 0){
        printf("Error creating pipe\n");
        exit(1);
    }

    // create a fork
    // this fork is a child process which is a copy of the parent process that created it.
    pid = fork();
    // check the process id
    if (pid < 0){
        printf("Fork failed!\n");
    } else if (pid == 0){
        printf("I am the child process!\n");
    } else {
        printf("I am the parent process!\n");
    }


    return 0;
}