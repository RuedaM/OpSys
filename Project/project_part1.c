/* project.c */

/*
 * TERMINAL COMMANDS:
 * gcc -Wall -Werror -Wextra -g -o project_part1.out project_part1.c
 * gcc -Wall -Werror -Wextra -g -o project_part1.out -D DEBUG_MODE project_part1.c
 * valgrind -s ./project_part1.out <ARGS HERE>
 * ARGS:
 * 3 1 32 0.001 1024 4 0.75 256
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



char (*gen_IDs(int n))[3];

struct Process{
    char ID[2];
};


int main(int argc, char** argv){
#ifdef DEBUG_MODE
    printf("=Check Arg Input=   // ");
    for(int i=1 ; i<=8 ; i++){printf("%s // ",*(argv+i));}
    printf("\n");
#endif
    setbuf(stdout, NULL);

    // Command Line Args
    if (argc!=9){ // (1 .out file + 8 inputs = 9 args)
        perror("ERR: invalid number of arguments");
        perror("USAGE: ./hw2.out <Proc_Count> <CPU-Bound_Procs> <IntArr_Times_Seed> <Lambda> <RandNumber_Upper_Bound> <t_cs> <alpha> <t_slice>");
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]); // number of processes to simulate
    int n_cpu = atoi(argv[2]); // number of processes that are CPU-bound
    int seed = atoi(argv[3]); // seed for the pseudo-random number sequence
    float lambda = atof(argv[4]); // paramter in (1/lambda) for average rand. value generated for exponential distribution for interarrival times
    int bound = atoi(argv[5]); // upper bound for pseudo-random numbers for exponential distribution ^
    int t_cs = atoi(argv[6]); // time, (in ms), that it takes to perform a context switch
    float alpha = atof(argv[7]); // for JF and SRT algorithms
    int t_slice = atoi(argv[8]); // time slice value in ms for RR algorithm
#if DEBUG_MODE
    printf("Processes: %d\n", n); printf("CPU-bound: %d\n", n_cpu); printf("Seed: %d\n", seed); printf("lambda: %f\n", lambda);
    printf("Upper Bound: %d\n", bound); printf("t_cs: %d\n", t_cs); printf("alpha: %f\n", alpha); printf("t_slice: %d\n", t_slice);
    printf("\n\n");
#endif

    if(t_cs<=0 && (t_cs%2)!=0) {perror("ERR: t_cs must be a positive even integer"); return EXIT_FAILURE;}
    if(t_slice<=0) {perror("ERR: t_slice must be a positive integer"); return EXIT_FAILURE;}

    printf("<<< -- process set (n=%d) with %d CPU-bound process\n", n, n_cpu);
    printf("<<< -- seed=%d; lambda=%f; bound=%d\n", seed, lambda, bound);

    // Process ID generation
    char (*IDs)[3] = gen_IDs(n);
#if DEBUG_MODE
    for (int i=0 ; i<n ; i++) {printf("%c%c\n", IDs[i][0], IDs[i][1]);}
#endif
    // remember to free! free(IDs)
}





char (*gen_IDs(int n))[3]{
    char (*ret)[3] = malloc(n * sizeof(*ret)); //DYNAMIC.MEMORY
    int i = 0;

    for (char let='A' ; let<='Z' ; let++) {   // Loop A-Z
        for (char num='0' ; num<='9' ; num++) {   // Loop 0-9
            ret[i][0] = let;
            ret[i][1] = num;
            ret[i][2] = '\0';
            i++;
            if (i==n) {return ret;}
        }
    }
    return ret;
}