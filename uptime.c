#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    // call the uptime system call
    int ticks = uptime();
    // print the ticks
    printf("System has been up for %d ticks! \n", ticks);
    return 0;
}