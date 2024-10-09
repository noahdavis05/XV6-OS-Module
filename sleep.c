#include "kernel/types.h"
#include "user/user.h"


int main(int argc, char *argv[]){
    // check the user has only passed one arg
    if (argc != 2){
        printf("Time is required!\n");
        return 1;
    }
    // convert the string to an int
    int ticks = atoi(argv[1]);
    // call the system call
    sleep(ticks);
    return 0;
}