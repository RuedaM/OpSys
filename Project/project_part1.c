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


float next_exp(float lambda, int bound, char* rounding, int only_drand);



int main(int argc, char** argv){
    
    setbuf(stdout, NULL);

#ifdef DEBUG_MODE
    printf("=Check Arg Input= // "); for(int i=1 ; i<=8 ; i++){printf("%s // ",*(argv+i));} printf("\n");
#endif
        
    if(argc!=9){ // (1 .out file + 8 inputs = 9 args)
        perror("ERR: invalid number of arguments");
        perror("USAGE: ./hw2.out <Proc_Count> <CPU-Bound_Procs> <IntArr_Times_Seed> <Lambda> <RandNumber_Upper_Bound> <t_cs> <alpha> <t_slice>");
        return EXIT_FAILURE;
    }

    // Command Line Arg Storage+Setting
    int n = atoi(argv[1]); // number of processes to simulate
    int n_cpu = atoi(argv[2]); // number of processes that are CPU-bound
    int seed = atoi(argv[3]); // seed for the pseudo-random number sequence
    float lambda = atof(argv[4]); // paramter in (1/lambda) for average rand. value generated for exponential distribution for interarrival times
    int bound = atoi(argv[5]); // upper bound for pseudo-random numbers for exponential distribution ^
    int t_cs = atoi(argv[6]); // time, (in ms), that it takes to perform a context switch
    float alpha = atof(argv[7]); // for JF and SRT algorithms
    int t_slice = atoi(argv[8]); // time slice value in ms for RR algorithm
#if DEBUG_MODE
    printf("Arg verification:\n");
    printf("  Processes: %d", n); printf("\t\tCPU-bound: %d\n", n_cpu);
    printf("  Seed: %d", seed); printf("\t\tlambda: %f\n", lambda);
    printf("  Upper Bound: %d", bound); printf("\tt_cs: %d\n", t_cs);
    printf("  alpha: %f", alpha); printf("\tt_slice: %d\n", t_slice);
    printf("\n");
#endif

    // Command Line Arg Error Checking
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

    // Terminal Output
    printf("<<< -- process set (n=%d) with %d CPU-bound process\n", n, n_cpu);
    printf("<<< -- seed=%d; lambda=%f; bound=%d\n", seed, lambda, bound);
#if DEBUG_MODE
    printf("\n");
#endif

    // Process ID Generation
    char** IDs = gen_IDs(n);
    // Remember to free in some way: free(IDs)
    // (Maybe loop through all processes and free each ID)
#if DEBUG_MODE
    printf("Generated IDs: // "); for (int i=0 ; i<n ; i++) {printf("%s // ", IDs[i]);} printf("\n\n");
#endif

    // Process Generation TEMPLATE
    srand48(seed);
    struct Process* allProcesses = calloc(n, sizeof(struct Process)); //DYNAMIC.MEMORY
    // Remember to free in some way: free(allProcesses)
    for (int i=0 ; i<n ; i++){
        int binding; if(i<n_cpu) {binding=0;} else {binding=1;}

        float arrivalTime = next_exp(lambda, bound, "floor", 0);
        
        int cpuBurstCount = next_exp(lambda, bound, "ceil", 1)*32; //TO-DO: ensure this ceil() still follows upper bound
        
        int* cpuBurstTimes = calloc(cpuBurstCount, sizeof(int));
        int* ioBurstTimes = calloc(cpuBurstCount-1, sizeof(int));
        int cpuBurstTime, ioBurstTime;
        // For all same-index CPU and I/O bursts
        for (int i=0 ; i<cpuBurstCount-1 ; i++){
            if(binding==0){ //CPU-bound
                cpuBurstTime = next_exp(lambda, bound, "ceil", 0)*4;
                ioBurstTime = next_exp(lambda, bound, "ceil", 0);
            }
            else if(binding==1){ //I/O-bound
                cpuBurstTime = next_exp(lambda, bound, "ceil", 0);
                ioBurstTime = next_exp(lambda, bound, "ceil", 0)*8;
            }
            cpuBurstTimes[i] = cpuBurstTime; ioBurstTimes[i] = ioBurstTime;
        }
        // For the last CPU burst
        if(binding==0) {cpuBurstTime = next_exp(lambda, bound, "ceil", 0)*4;}
        else if(binding==1) {cpuBurstTime = next_exp(lambda, bound, "ceil", 0);}
        cpuBurstTimes[cpuBurstCount-1] = cpuBurstTime;

#if DEBUG_MODE
        printf("Building Process %s:\n", IDs[i]);
        printf("  Current binding: ");
        if (binding==0) {printf("CPU\n");} else {printf("I/O\n");}
        printf("  Generated interarrival_time: %f\n", arrivalTime);
        printf("  CPU Bursts: %d --- I/O Bursts: %d\n", cpuBurstCount, cpuBurstCount-1);
        printf("  CPU Burst Times: |");
        for (int i=0 ; i<cpuBurstCount ; i++){printf("%d] %d |", i, cpuBurstTimes[i]);} printf("\n");
        printf("  I/O Burst Times: |");
        for (int i=0 ; i<cpuBurstCount-1 ; i++){printf("%d] %d |", i, ioBurstTimes[i]);} printf("\n");
        printf("\n");
#endif
        
        struct Process proc = {IDs[i], 2, binding, arrivalTime, cpuBurstCount, cpuBurstTimes, ioBurstTimes};
        allProcesses[i] = proc;
    }
#if DEBUG_MODE
for (int i=0 ; i<n ; i++){
    printf("Process %s verified\n", allProcesses[i].ID);
}
#endif







    return EXIT_SUCCESS;
}




float next_exp(float lambda, int bound, char* rounding, int only_drand){
    double r, x;
    while(1){
        r = drand48();   // uniform dist [0.00,1.00)

        if(only_drand){   // "Function ONLY asking for next value of just drand48()"
            if(r>bound) {continue;}
            else {return r;}
        }

        x = (-log(r)/lambda);   // generate next pseudo-random value
        // (Note: log() = natural log)
        
        if(strcmp(rounding, "ceil")) {x = ceil(x);}
        if(strcmp(rounding, "floor")) {x = floor(x);}

        if(x>bound) {continue;}
        else {break;}
    }

    return x;
}