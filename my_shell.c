#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"


/* Read a line of characters from stdin. */
int getcmd(char *buf, int nbuf) {
    // Print >>> then clear buffer
    printf(">>> ");
    memset(buf, 0, nbuf);

    // Read the user's input into buf
    gets(buf, nbuf);
    if (buf[0] == 0) {
        return -1;  // Return -1 if input is empty
    }

    // Remove the newline character if present
    for (int i = 0; i < nbuf; i++) {
        if (buf[i] == '\n') {
            buf[i] = '\0';  // Replace \n with null terminator
            break;  // Exit the loop after replacing
        }
    }

    return 0;  // Return 0 on success
}


/*
  A recursive function which parses the command
  at *buf and executes it.
*/
__attribute__((noreturn))
void run_command(char *buf, int nbuf, int *pcp) {
  
  // check for a ; and make a new string after the ;
  // then can recursively call this function with that string

  //printf("%s command 2\n", commands[1]);
  /* Useful data structures and flags. */
  char *arguments[10]; // Command arguments
  int numargs = 0;
  int redirection_left = 0;
  int redirection_right = 0;
  int redirecttion_append_right = 0;
  char *file_name_l = 0;
  char *file_name_r = 0;
  int pipe_cmd = 0;
  int sequence_cmd = 0;
  // for seqential commands
  int sequential = 0;
  char *second_command = 0;
  
  

  /* Parse the command character by character */
  
  int ws = 0; // word start index
  //int start = 0;

  /* Loop through the input buffer to extract arguments */

  for (int i = 0; i < nbuf; i++) {
    if (sequential || pipe_cmd){
      buf[i] = '\0';
    }
    else if (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\0') {
        if (ws != i) { // Ensure we aren't capturing empty arguments
            buf[i] = '\0'; // Null-terminate the current argument
            if (numargs < 10) {
                arguments[numargs++] = &buf[ws]; // Add the starting index of the argument
            }
        } 
        ws = i + 1; // Update the word start to the next character
    } else if (buf[i] == '>'){
      buf[i] = '\0';
      // check if there is another '>' afterwards
      if (buf[i+1] == '>'){
        //this means redirect and append the content
        redirecttion_append_right = 1;
        i++;
      } else {
        redirection_right = 1;
      }
      

      // check if there is a space after the '>'
      if (buf[i+1] != ' '){
        file_name_r = &buf[i+1];
      } else {
        while (buf[i + 1] == ' ') i++; // Skip any spaces after '>'
        file_name_r = &buf[i];
        // check if buf[i] is a space, if it is the next char is start of filename
        if (buf[i] == ' '){
          file_name_r = &buf[i+1];
        }
      }
      continue; // Skip the rest of the loop
      
    } else if (buf[i] == '<'){
      buf[i] = '\0';
      redirection_left = 1;
      if (buf[i+1] != ' '){
        file_name_l = &buf[i+1];
      } else {
        while (buf[i + 1] == ' ') i++; // Skip any spaces after '>'
        file_name_l = &buf[i];
        // check if buf[i] is a space, if it is the next char is start of filename
        if (buf[i] == ' '){
          file_name_l = &buf[i+1];
        }
      }
      continue; // Skip the rest of the loop
      
    } else if ((buf[i] == ';' && sequential == 0) || (buf[i] == '|' && pipe_cmd == 0)){
      if (buf[i] == ';'){
        sequential = 1;
      } else {
        pipe_cmd = 1;
      }
      

      // Calculate the length of the second command
      int j = 0;
      while (buf[i + 1 + j] == ' ') {
          j++;  // Skip spaces
      }
      
      // Calculate the length of the second command
      int second_command_length = strlen(&buf[i + 1 + j]) + 1; // +1 for null terminator

      // Allocate memory for second_command
      second_command = (char *)malloc(second_command_length);
      if (second_command == 0) {
          printf("Memory allocation failed\n");
          exit(1);
      }

      // Copy the part of buf after ';'
      strcpy(second_command, &buf[i + 1 + j]);

      // Optionally, trim any leading spaces in the second command
      //printf("First command: %s\n", buf);
      //printf("Second command: %s\n", second_command);

      // Null-terminate the first command (optional, to split the string)
      buf[i] = '\0';  // This will separate the first command from the second
    }
}

// Ensure the last argument is captured if the buffer ends without a space
if (ws < nbuf) {
    arguments[numargs++] = &buf[ws];
}
arguments[numargs] = 0; // Null-terminate the argument array


  /* Handle sequence commands (e.g., commands separated by ';') */
  if (sequence_cmd) {
    sequence_cmd = 0;
    if (fork() != 0) {
      wait(0);
      // Handle the next command after ';'
    }
  }

  // handle pipe commands here
  if (pipe_cmd){
    // create a pipe to pass data between
    int pipefd[2]; // holds pipe's read and write ends

    // create the pipe
    if (pipe(pipefd) == -1){
      printf("Pipe failed\n");
      exit(1);
    }

    //printf("Executing command: %s\n", arguments[0]);

    // fork for the left hand side of the command
    if (fork() == 0){
      // redirect the stdout to the write end of the pipe
      close(pipefd[0]); // read end of pipe
      close(1); // close stdout
      dup(pipefd[1]); // duplicate pipes write end to stdout
      close(pipefd[1]); // close the write end after duplication.

      // execute left hand side
      if (exec(arguments[0], arguments) == -1){
        printf("Exec failed\n");
        exit(1);
      }
    } else {
      // in parent process after forking left command

      // close write end of pipe as only needs to be read now
      close(pipefd[1]);
      if (second_command != 0){
        if (fork() == 0){
          // in the right side command now
          close(0); // close stding
          dup(pipefd[0]);
          close(pipefd[0]);

          // recursively run this command
          run_command(second_command, strlen(second_command), pipefd);
          free(second_command);
        }
      }
      // close read end of pipe in parent
      close(pipefd[0]);

      wait(0);
      if (second_command != 0){
        wait(0);
      }
    }



  }

  /* If this is a CD command, handle it separately */
  else if (strcmp(arguments[0], "cd") == 0) {
    if (numargs < 2) {
      printf("cd: missing argument\n");
    } else if (chdir(arguments[1]) != 0) {
      printf("cd: %s: No such directory\n", arguments[1]);
    }
  } else {
    /* For other commands, fork a child process to execute */
    if (fork() == 0) {
      // check redirection
      if (redirection_right){
        
        // open the file
        int fd = open(file_name_r, O_WRONLY | O_CREATE | O_TRUNC);
        if (fd < 0){
          printf("Failed to open %s for writing\n", file_name_r);
          exit(1);
        }
        // close the standard output, then duplicate output to file
        close(1);
        dup(fd);
        close(fd);
        
      }
      if (redirecttion_append_right){
        // open the file for writing
        int fd = open(file_name_r, O_RDWR);
        if (fd >= 0){
          // read to the end of the file to then append
          char buffer[1024];
          int n;
          while ((n=read(fd, buffer, sizeof(buf))) > 0); // keep reading until the end of fole
          // now at the edn write to the file
          close(1);
          dup(fd);
          close(fd);
        }
      }
      if (redirection_left) {
        // Open the file for reading
        //printf("%s\n",file_name_l);
        int fd = open(file_name_l, O_RDONLY);
        if (fd < 0) {
            printf("Failed to open %s for reading\n", file_name_l);
            exit(1);
        }
        
        // Close stdin and redirect it to the file
        close(0); // Close the original stdin
        
        dup(fd);
          // Duplicate fd onto stdin (0)
        close(fd); // Close the file descriptor as it's no longer needed
        // need to work out how many args before '<'
        arguments[numargs - 1] = 0;


        
    }

      // Child process
      //
      if (exec(arguments[0], arguments) < 0);
      // If exec fails
      printf("Unknown command: %s\n", arguments[0]);
      exit(1);
    } else {
      // Parent process
      wait(0);
      if (sequential){
        run_command(second_command, strlen(second_command), pcp);
        free(second_command);
      }
      //fprintf(1, "Executing command:arg0 %s, arg1 %s \n",arguments[0], arguments[1]);
    }
  }
  
  /* Exit after command execution */
  exit(0);
}



int main(void) {
    static char buf[100];
    int pcp[2];
    pipe(pcp);

    // Read and run input commands
    // this main function has been made just to ensure cd works, as it needs to be executed outside fork
    
    while (getcmd(buf, sizeof(buf)) >= 0) {
        char *arguments[10];  // Array to hold command arguments
        int numargs = 0;
        int ws = 0;

        // Loop through the input buffer to extract arguments and run
        for (int i = 0; i < strlen(buf); i++) {
            if (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\0') {
                if (ws != i) {  // Ensure we aren't capturing empty arguments
                    buf[i] = '\0';  // Null-terminate the current argument
                    if (numargs < 10) {
                        
                        arguments[numargs] = &buf[i + 1];  // Add the argument to the list
                        numargs++;
                    }

                }
                ws = i + 1;  // Update the word start to the next character
            }
        }

        // Make sure we properly null-terminate the argument list
        arguments[numargs] = 0;



        // Handle the "cd" command in the parent process
        if (numargs > 0 && strcmp(buf, "cd") == 0) {
            if (numargs < 1) {
                printf("cd: missing argument\n");
            } else if (chdir(arguments[0]) != 0) {
                printf("cd: %s: No such directory\n", arguments[0]);
            }
            continue;  // Skip the rest of the loop to avoid forking for "cd"
        }
        
        // For all other commands, fork a new process
        if (fork() == 0) {
            // In the child process, execute the command
            run_command(buf, 100, pcp);
        }

        int child_status;
        wait(&child_status);  // Wait for the child process to complete
    }

    exit(0);
}
