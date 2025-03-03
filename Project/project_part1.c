/* project.c */

/*
 * TERMINAL COMMANDS:
 * gcc -Wall -Werror -Wextra -g -o project.out project.c
 * gcc -Wall -Werror -Wextra -g -o project.out -D DEBUG_MODE project.c
 * valgrind -s ./project.out <ARGS HERE>
 * ARGS:
 * [...]
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>



int main(int argc, char** argv){
#ifdef DEBUG_MODE
    for(int i=1 ; i<=4 ; i++){printf("Arg #%d: %s // ", i, *(argv+i));}
    printf("\n");
#endif
    setbuf(stdout, NULL);

    // Command Line Args
    if (argc!=9){ // (1 .out file + 8 inputs = 9 args)
        perror("ERR: invalid number of arguments");
        perror("USAGE: ./hw2.out <Proc_Count> <CPU-Bound_Procs> <IntArr_Times_Seed> <Lambda> <RandNumber_Upper_Bound> <t_cs> <alpha> <t_slice>");
        return EXIT_FAILURE;
    }

    int n = atoi(*(argv+1)); // number of processes to simulate
    int n_cpu = atoi(*(argv+2)); // number of processes that are CPU-bound
    int seed = atoi(*(argv+3)); // seed for the pseudo-random number sequence
    int lambda = atoi(*(argv+4)); // paramter in (1/lambda) for average rand. value generated for exponential distribution for interarrival times
    int upBound = atoi(*(argv+5)); // upper bound for pseudo-random numbers for exponential distribution ^
    int t_cs = atoi(*(argv+6)); // time, (in ms), that it takes to perform a context switch
    int alpha = atoi(*(argv+7)); // for JF and SRT algorithms
    int t_slice = atoi(*(argv+8)); // time slice value in ms for RR algorithm

    if (t_slice<=0) {
        perror("ERR: t_slice must be positive");
        return EXIT_FAILURE;
    }
}