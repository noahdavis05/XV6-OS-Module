#include "kernel/types.h"
#include "user/user.h"

int main(void){
    fprintf(1,"Note: Unix v6 was released in the year %d\n", getyear());
    return 0;
}