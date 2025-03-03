/* fork-multiple-children.c */

/*
 * CTRL-C sends a SIGINT to all processes in the group:
 * bash$ kill -SIGINT -<parent>
 * 
 * To only send a signal to the parent process:
 * bash$ kill -SIGINT <parent>
 * (Note the lack of a flag on <parent>)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>


int childprocess();
void parent_process(int children);



int main(int argc, char** argv){

    printf("PARENT: My PID is %d\n", getpid());

    if (argc!=2){
        fprintf(stderr, "ERR: Invalid arguments\n");
        fprintf(stderr, "USAGE: %s <#-of-children>\n", *(argv+0));
        return EXIT_FAILURE;
    }

    int children = atoi(*(argv+1));

    // Create child processes
    for (int i=0 ; i<children ; i++){

        pid_t p = fork();
        if (p==-1){ perror("fork() failed"); return EXIT_FAILURE; }

        if (p==0){
            srand(i + time(NULL));
            return child_process();
            // Be sure to EXIT child process here...
        }
    }

    parent_process(children);

    return EXIT_SUCCESS;
}



// Each child process will sleep for a short time
int child_process(){
    //srand(time(NULL));
    int t = 10 + (rand() % 21);   // [10,30] seconds

    printf("CHILD %d: I'm gonna nap for %d seconds...\n", getpid(), t);
    sleep(t);
    printf("CHILD %d: I'm awake!\n", getpid());
    return t;
}

void parent_process(int children){
    int status;

    printf("PARNT: I'm waiting for my child process to exit\n");
    while (children>0){
        pid_t child_pid = waitpid(-1, &status, 0);   // BLOCKING
        //                        ^^
        //              wait for any of my child processes...just whatever's first

        children--;

        printf( "PARENT: child process %d terminated...\n", child_pid );
        if (WIFSIGNALED(status) ){
            printf( "PARENT: ...abnormally (killed by a signal)\n" );
        }
        else if (WIFEXITED(status)){
            int exit_status = WEXITSTATUS( status );
            printf( "PARENT: ...normally with exit status %d\n", exit_status );
        }
    }

    printf("PARENT: All done");
}