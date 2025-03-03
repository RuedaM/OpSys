/* dealy.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv){
    printf("PID %d: argc is %d\n", getpid(), argc);
    printf("PID %d: The parent process is PID %d\n", getpid(), getppid());
    printf("PID %d: Calculating something very important...\n", getpid());
    
    sleep(30);

    printf("PID %d: ALL done --- terminating...\n", getpid());

    return EXIT_SUCCESS;
}