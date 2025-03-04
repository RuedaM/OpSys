/* project.c */

/*
 * TERMINAL COMMANDS:
 * gcc -Wall -Werror -Wextra -g -o project_part1.out project_part1.c -lm
 * gcc -Wall -Werror -Wextra -g -o project_part1.out -D DEBUG_MODE project_part1.c -lm
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
#include "project_part1.h"


float next_exp(float lambda, int bound);



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

    // Command Line Argument Error Checking
    if(n<=0) {perror("ERROR: n must be a positive integer"); return EXIT_FAILURE;}
    // Next few error checks are assumptions that they must be a positive integer. Can change/remove later when more test cases/examples are given
    // --------------------------- REMOVE THESE AS APPROPRIATE ---------------------------
    if(n_cpu<=0) {perror("ERROR: n_cpu must be a positive integer"); return EXIT_FAILURE;}
    if(seed<=0) {perror("ERROR: seed must be a positive integer"); return EXIT_FAILURE;}
    if(lambda<=0) {perror("ERROR: lambda must be a positive float"); return EXIT_FAILURE;}
    if(bound<=0) {perror("ERROR: bound must be a positive integer"); return EXIT_FAILURE;}
    // -----------------------------------------------------------------------------------
    if(t_cs<=0 && (t_cs%2)!=0) {perror("ERROR: t_cs must be a positive even integer"); return EXIT_FAILURE;}
    if(alpha<0 || alpha>1) {perror("ERROR: alpha must be in range [0,1]}"); return EXIT_FAILURE;}
    if(t_slice<=0) {perror("ERROR: t_slice must be a positive integer"); return EXIT_FAILURE;}

    printf("<<< -- process set (n=%d) with %d CPU-bound process\n", n, n_cpu);
    printf("<<< -- seed=%d; lambda=%f; bound=%d\n", seed, lambda, bound);


    // Process ID Generation
    char** IDs = gen_IDs(n);
    // Remember to free in some way: free(IDs)
    // (Maybe loop through all processes and free each ID)
#if DEBUG_MODE
    for (int i=0 ; i<n ; i++) {printf("%s\n", IDs[i]);}
#endif


    // Process Generation Template
    srand48(seed);
    // struct Process* allProcesses = calloc(n, sizeof(struct Process)); //DYNAMIC.MEMORY
    // Remember to free in some way: free(allProcesses)
    for (int i=0 ; i<n ; i++){

        int binding; if (i<n_cpu) {binding = 0;} else {binding = 1;}
        
        float interarrival_time = next_exp(lambda, bound);
        
        int CPUBurstCount = ceil(drand48())*32; //TO-DO: ensure this ceil() still follows upper bound
        for (int i=0 ; i<CPUBurstCount ; i++){
            int CPUBurst = next_exp(lambda, bound);
            int IOBurst = next_exp(lambda, bound)*8;
        }
        /* 
         * For each of these CPU bursts, identify the CPU burst time and the I/O burst time as the
         * “ceiling” of the next two random numbers in the sequence given by next_exp(); multiply
         * the I/O burst time by 8 such that I/O burst time is close to an order of magnitude longer
         * than CPU burst time; as noted above, for CPU-bound processes, multiply the CPU burst
         * time by 4 and divide the I/O burst time by 8 (i.e., do not bother multiplying the original
         * I/O burst time by 8 in this case); for the last CPU burst, do not generate an I/O burst time
         * (since each process ends with a final CPU burst)
         */

#if DEBUG_MODE
        printf("Current binding: ");
        if (binding==0) {printf("CPU\n");} else {printf("I/O\n");}
        printf("Generated interarrival_time: %f\n", interarrival_time);
#endif
        
        // struct Process proc = {IDs[i], binding};
        // allProcesses[i] = proc;
    }

    return EXIT_SUCCESS;
}




float next_exp(float lambda, int bound){
    int min = 0, max = 0, sum = 0;
    int iterations = 1000000; // Arbitrary, as long as it's large

    for (int i=0 ; i<iterations ; i++){
        double r = drand48();  // uniform dist [0.00,1.00)

        double x = (-log(r)/lambda);   // generate the next pseudo-random value x
        // (Note: log() = natural log)
        
        if (x>bound) {i--; continue;}

#if DEEP_DEBUG_MODE
        /* display the first 20 pseudo-random values */
        if (i<20) {printf("x is %lf\n", x);}
#endif

        sum += x;
        if (i==0 || x<min) {min=x;}
        if (i==0 || x>max) {max=x;}
    }
    return (sum/iterations);
}