#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"


/* Read a line of characters from stdin. */
int getcmd(char *buf, int nbuf) {
  // print >>> then clear buffer
  printf(">>> ");
  memset(buf,0,nbuf);

  // read the user's input into buf
  gets(buf, nbuf);
  if (buf[0] == 0){
    return -1;
  }

  return 0;
}

/*
  A recursive function which parses the command
  at *buf and executes it.
*/
__attribute__((noreturn))
void run_command(char *buf, int nbuf, int *pcp) {
  // split the command up into commands, if multiple

  // check for  ';' and split into commands here
  /* Useful data structures and flags. */
  char *arguments[10]; // Command arguments
  int numargs = 0;
  int redirection_left = 0;
  int redirection_right = 0;
  int redirecttion_append_right = 0;
  char *file_name_l = 0;
  char *file_name_r = 0;
  //int pipe_cmd = 0;
  int sequence_cmd = 0;

  /* Parse the command character by character */
  int i = 0;
  int ws = 0; // word start index
  //int start = 0;

  /* Loop through the input buffer to extract arguments */
  for (i = 0; i < nbuf; i++) {
    if (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\0') {
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

  /* If this is a CD command, handle it separately */
  if (strcmp(arguments[0], "cd") == 0) {
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
  while(getcmd(buf, sizeof(buf)) >= 0) {
    if(fork() == 0) {
      run_command(buf, 100, pcp);
    }
    int child_status;
    wait(&child_status);
  }

  exit(0);
}
