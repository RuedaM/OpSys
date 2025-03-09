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



int main(int argc, char** argv){
    
    setbuf(stdout, NULL);

#ifdef DEBUG_MODE
    printf("=Check Arg Input= // "); for(int i=1 ; i<=8 ; i++){printf("%s // ",*(argv+i));} printf("\n");
#endif
        
    if(argc!=9){ // (1 .out file + 8 inputs = 9 args)
        fprintf(stderr, "ERROR: Invalid number of arguments\n");
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
    if(n<=0) {fprintf(stderr, "ERROR: n must be a positive integer\n"); return EXIT_FAILURE;}
    // Next few error checks are assumptions that they must be a positive integer. Can change/remove later when more test cases/examples are given
    // --------------------------- REMOVE THESE AS APPROPRIATE ---------------------------
    if(n_cpu<=0) {fprintf(stderr, "ERROR: n_cpu must be a positive integer\n"); return EXIT_FAILURE;}
    if(seed<=0) {fprintf(stderr, "ERROR: seed must be a positive integer\n"); return EXIT_FAILURE;}
    if(lambda<=0) {fprintf(stderr, "ERROR: lambda must be a positive float\n"); return EXIT_FAILURE;}
    if(bound<=0) {fprintf(stderr, "ERROR: bound must be a positive integer\n"); return EXIT_FAILURE;}
    // -----------------------------------------------------------------------------------
    if(t_cs<=0 && (t_cs%2)!=0) {fprintf(stderr, "ERROR: t_cs must be a positive even integer\n"); return EXIT_FAILURE;}
    if(alpha<0 || alpha>1) {fprintf(stderr, "ERROR: alpha must be in range [0,1]}\n"); return EXIT_FAILURE;}
    if(t_slice<=0) {fprintf(stderr, "ERROR: t_slice must be a positive integer\n"); return EXIT_FAILURE;}

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

    // Process Generation
    struct Process* allProcesses = gen_procs(IDs, seed, n, n_cpu, lambda, bound);
#if DEBUG_MODE
for (int i=0 ; i<n ; i++) {printf("Process %s verified\n", allProcesses[i].ID);}
#endif
    // Process Generation Terminal Output
    for(int i=0 ; i<n ; i++){ //For every process...
        printf("CPU-bound process %s: arrival time %dms; %d CPU bursts:\n", allProcesses[i].ID, allProcesses[i].arrivalTime, allProcesses[i].cpuBurstCount);
        for(int j=0 ; j<allProcesses[i].cpuBurstCount-1 ; j++){printf("==> CPU burst %dms ==> I/O burst %dms\n", allProcesses[i].cpuBurstTimes[j], allProcesses[i].ioBurstTimes[j]);}
        printf("==> CPU burst %dms\n\n", allProcesses[i].cpuBurstTimes[allProcesses[i].cpuBurstCount-1]);
    }
    
    // Calculations for simout.txt
    float cpuBoundAvgCPUBurstTime, ioBoundAvgCPUBurstTime, totalAvgCPUBurstTime, cpuBoundAvgIOBurstTime, ioBoundAvgIOBurstTime, totalAvgIOBurstTime;
    for(int i=0 ; i<n ; i++){ //For every process...
        
        int cpuBoundAvgCPUBurstTime_1Proc = 0, ioBoundAvgCPUBurstTime_1Proc = 0, totalAvgCPUBurstTime_1Proc = 0;
        for(int j=0 ; j<allProcesses[i].cpuBurstCount ; j++){ //Looking at CPU burst times
            if (allProcesses[i].binding==0) {cpuBoundAvgCPUBurstTime_1Proc += allProcesses[i].cpuBurstTimes[j];} //CPU-bound
            if (allProcesses[i].binding==1) {ioBoundAvgCPUBurstTime_1Proc += allProcesses[i].cpuBurstTimes[j];} //I/O-Bound
            totalAvgCPUBurstTime_1Proc += allProcesses[i].cpuBurstTimes[j];
        }
        cpuBoundAvgCPUBurstTime_1Proc /= allProcesses[i].cpuBurstCount;
        ioBoundAvgCPUBurstTime_1Proc /= allProcesses[i].cpuBurstCount;
        totalAvgCPUBurstTime_1Proc /= allProcesses[i].cpuBurstCount;

        int cpuBoundAvgIOBurstTime_1Proc = 0, ioBoundAvgIOBurstTime_1Proc = 0, totalAvgIOBurstTime_1Proc = 0;
        for(int j=0 ; j<allProcesses[i].cpuBurstCount-1 ; j++){ //Looking at I/O burst times
            if (allProcesses[i].binding==0) {cpuBoundAvgIOBurstTime_1Proc += allProcesses[i].ioBurstTimes[j];} //CPU-Bound
            if (allProcesses[i].binding==1) {ioBoundAvgIOBurstTime_1Proc += allProcesses[i].ioBurstTimes[j];} //I/O-Bound
            totalAvgIOBurstTime_1Proc += allProcesses[i].ioBurstTimes[j];
        }
        cpuBoundAvgIOBurstTime_1Proc /= (allProcesses[i].cpuBurstCount-1);
        ioBoundAvgIOBurstTime_1Proc /= (allProcesses[i].cpuBurstCount-1);
        totalAvgIOBurstTime_1Proc /= (allProcesses[i].cpuBurstCount-1);

        cpuBoundAvgCPUBurstTime += cpuBoundAvgCPUBurstTime_1Proc;
        ioBoundAvgCPUBurstTime += ioBoundAvgCPUBurstTime_1Proc;
        totalAvgCPUBurstTime += totalAvgCPUBurstTime_1Proc;
        cpuBoundAvgIOBurstTime += cpuBoundAvgIOBurstTime_1Proc;
        ioBoundAvgIOBurstTime += ioBoundAvgIOBurstTime_1Proc;
        totalAvgIOBurstTime += totalAvgIOBurstTime_1Proc;
    }
    cpuBoundAvgCPUBurstTime /= n_cpu;
    ioBoundAvgCPUBurstTime /= (n-n_cpu);
    totalAvgCPUBurstTime /= n;
    cpuBoundAvgIOBurstTime /= n_cpu;
    ioBoundAvgIOBurstTime /= (n-n_cpu);
    totalAvgIOBurstTime /= n;

    // simout.txt Initialization
    int fd = open("simout.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    // Remember to close in some way: close(fd)
    if (fd==-1) {fprintf(stderr, "ERROR: open() failed\n"); return EXIT_FAILURE;}
    
    ssize_t bytesWritten; char toWrite[200];

    sprintf(toWrite, "-- number of processes: %d\n", n);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- number of CPU-bound processes: %d\n", n_cpu);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- number of I/O-bound processes: %d\n", n-n_cpu);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}

    // simout.txt Output
    sprintf(toWrite, "-- CPU-bound average CPU burst time: %.3f ms\n", cpuBoundAvgCPUBurstTime);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- I/O-bound average CPU burst time: %.3f ms\n", ioBoundAvgCPUBurstTime);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- overall average CPU burst time: %.3f ms\n", totalAvgCPUBurstTime);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- CPU-bound average I/O burst time: %.3f ms\n", cpuBoundAvgIOBurstTime);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- I/O-bound average I/O burst time: %.3f ms\n", ioBoundAvgIOBurstTime);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- overall average I/O burst time: %.3f ms\n\n", totalAvgIOBurstTime);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}







    return EXIT_SUCCESS;
}