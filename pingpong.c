#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main() {
    int pipe1[2]; // Pipe for parent to child
    int pipe2[2]; // Pipe for child to parent
    unsigned char buf;     // Buffer for messages
    int pid;
    // chars to be sent from parent to child and vice versa
    char c1 = 'Q';
    char c2 = 'P';


    // Create the first pipe (parent to child)
    if (pipe(pipe1) < 0) {
        fprintf(2, "pipe1 failed\n");
        exit(1);
    }

    // Create the second pipe (child to parent)
    if (pipe(pipe2) < 0) {
        fprintf(2, "pipe2 failed\n");
        exit(1);
    }

    // Fork the process
    pid = fork();

    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) { // Child process
        close(pipe1[1]); // Close the write end of the first pipe
        close(pipe2[0]); // Close the read end of the second pipe

        // Read from the first pipe
        if (read(pipe1[0], &buf, 1) == 1) { // Check that exactly 1 byte is read
            printf("%d: received ping ", getpid()); // Print received message
            write(1, &buf, 1);
            write(1, "\n",1);
        } else {
            fprintf(2, "child: read failed\n");
        }

        // Send pong back through the second pipe
        if (write(pipe2[1], &c1 , 1) != 1) { // Check that the write succeeds
            fprintf(2, "child: write failed\n");
        } else {
            //printf("Child sent pong \n");
        }

        close(pipe1[0]); // Close the read end of the first pipe
        close(pipe2[1]); // Close the write end of the second pipe
        exit(0); // Exit the child process
    } else { // Parent process
        close(pipe1[0]); // Close the read end of the first pipe
        close(pipe2[1]); // Close the write end of the second pipe

        // Send a byte to the child
        if (write(pipe1[1], &c2 , 1) != 1) { // Check that the write succeeds
            fprintf(2, "parent: write failed\n");
        } else {
            
            //write(1,"Parent sent ping \n", 20 );  
            //write(1, &c2, 1);
             
        }
        
        // Wait for the child to finish
        wait(0); // Wait for the child to finish

        // Read the response from the second pipe
        if (read(pipe2[0], &buf, 1) == 1) { // Check that exactly 1 byte is read
            printf("%d: received pong ", getpid()); // Print received message
            write(1, &buf, 1);
            write(1, "\n",1);
        } else {
            fprintf(2, "parent: read failed\n");
        }

        close(pipe1[1]); // Close the write end of the first pipe
        close(pipe2[0]); // Close the read end of the second pipe
        exit(0); // Exit the parent process
    }
}
