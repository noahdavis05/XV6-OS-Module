#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]){
    // must have at least one arg
    if (argc < 2){
        fprintf(2, "Usage: xargs command\n");
        exit(1);
    }

    // now i must read the input characters one character at a time
    char buf[512];
    int n;
    int i = 0;
    // this while loop reads each character from the stdin (e.g. the command line the command before the pipe)
    // then whenever a new line is detected in this data
    // a fork is created and the new command from after the pipe is ran on each piece of data.
    while ((n = read(0,&buf[i], 1)) > 0){
        if (buf[i] == '\n'){
            // Process the line (fork, exec)
            buf[i] = 0;
            i = 0;

            // fork a child process
            if (fork() == 0){
                // Build the argument list for the exec() system call
                char *newargv[MAXARG];
                int j;

                // copy the command arguments
                for (j = 1; j < argc; j++){
                    newargv[j-1] = argv[j];
                }

                // add the input line as the last argument
                newargv[j-1] = buf;
                newargv[j] = 0;

                // execute the command using exec system call
                exec(newargv[0], newargv);
                fprintf(2,"exec failed\n");
                exit(1);
            } else {
                // parent waits for child to finish
                wait(0);
            }

            i = 0;
        } else {
            i++;
        }
    }


    return 0;
}