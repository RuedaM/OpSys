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
#include <time.h>

#include "project_part1.h"

#define SLEEP_TIME 1

// =====First Come First Serve (FCFS)=====
int FCFS(struct Process* allProcesses, int n, int t_cs){
    int time = 0;
    int FCFSFlag = 1;
    int csLeft = t_cs/2;
    
    struct Process* queue = calloc(0, sizeof(struct Process)); //DYNAMIC.MEMORY
    int queueLen = 0;

    struct Process cpuProc;
    int cpuIsRunning = 0;   // Boolean to see if a process is in CPU
    
    struct Queue* io = {NULL};
    // Remember to free in some way: free(IO)
    int ioLen = 0;

    int completedProcs = 0;

    printf("time %dms: Simulator started for FCFS", time);
    queue_status(queue, queueLen);
#if DEBUG_MODE
printf("===BEGINNING FCFS===\n");
#endif


    //======================================================================================================================
    while(FCFSFlag){
        //======================================================================================================================
#if DEBUG_MODE
printf("TIME: %d\n", time);
#endif

#if DEBUG_MODE
printf("Checking if process can be deleted or added to I/O...\n");
#endif
        if (cpuIsRunning && cpuProc.cpuBurstCurr==0){ // If CPU in-use and cpuProc's burst is done...
            if(csLeft==0){ // If done cs'ing...
                csLeft = t_cs/2; //Reset
                cpuIsRunning = 0; // "CPU no longer running a proccess"

                if ((cpuProc.idx+1)==cpuProc.cpuBurstCount){ // If CPUProc has no more CPU bursts...
                    printf(" End of process...\n");
                    cpuProc.state = 4; //state=TERMINATED
                    completedProcs += 1;

                    printf("time %dms: Process %s terminated", time, cpuProc.ID);
                    queue_status(queue, queueLen);
                }else{ // If CPUProc still has CPU bursts...    
                    printf("Into I/O...\n");
                    cpuProc.state = 3; //state==IN-I/O
                    cpuProc.ioBurstCurr = cpuProc.ioBurstTimes[cpuProc.idx];   // Establish current I/O burst time
                    queue_push(io, cpuProc);
                    ioLen += 1;

                    printf("time %dms: Process %s completed a CPU burst; %d bursts to go",
                        time, cpuProc.ID, (cpuProc.cpuBurstCount-(cpuProc.idx+1)));
                    queue_status(queue, queueLen);
                    printf("time %dms: Process %s switching out of CPU; blocking on I/O until time %dms",
                        time, cpuProc.ID, (time+(t_cs/2)+cpuProc.ioBurstCurr));
                    queue_status(queue, queueLen);
                }
            }
#if DEBUG_MODE
sleep(SLEEP_TIME);
#endif
        }


        //======================================================================================================================
#if DEBUG_MODE
printf("Checking if process can be added to CPU...\n");
#endif
        if (queueLen!=0 && !cpuIsRunning){ // If queue isn't empty and nothing is in the CPU... 
            if (csLeft==0){  // If done cs'ing...
                csLeft = t_cs/2; //Reset
                cpuProc = pop(queue, queueLen); // Get latest-in-queue process; this also resizes queue
                if (queue==NULL) {fprintf(stderr, "ERROR: realloc() failed\n"); return EXIT_FAILURE;}
                queueLen -= 1;
                cpuProc.state = 0; //state==IN-CPU
                cpuProc.cpuBurstCurr = cpuProc.cpuBurstTimes[cpuProc.idx]; // Establish current CPU burst time
                cpuIsRunning = 1;   // "CPU is now running a proccess"

                printf("time %dms: Process %s started using the CPU for %dms burst",
                    time, cpuProc.ID, cpuProc.cpuBurstCurr);
                queue_status(queue, queueLen);
            }  
#if DEBUG_MODE
sleep(SLEEP_TIME);
#endif        
        }


        //======================================================================================================================
#if DEBUG_MODE
printf("Checking if process can be added back to queue after I/O...\n");
#endif
    while(1){ // Until all 0-I/O-burst-time processes have been moved...
        if (ioLen!=0 && io->head->p.ioBurstCurr==0){ // If next-up-proc's I/O burst is finished...
            queue = realloc(queue, (queueLen+1)*sizeof(struct Process));
            if (!queue) {fprintf(stderr, "ERROR: realloc() failed\n"); return EXIT_FAILURE;}
            
            queue[queueLen] = queue_pop(io); // Remove next-up-proc, add to back of queue
            queue[queueLen].state = 1; //state=IN-QUEUE
            queue[queueLen].idx += 1; //Increase CPU and I/O bust times index
            queueLen += 1;
            ioLen -= 1;
            
            printf("time %dms: Process %s completed I/O; added to ready queue",
                time, queue[queueLen].ID);
            queue_status(queue, queueLen);
        }else{ // If next-up-proc's I/O burst isn't finished...
            break;
        }
#if DEBUG_MODE
sleep(SLEEP_TIME);
#endif
    }


        //======================================================================================================================
#if DEBUG_MODE
printf("Checking if arriving process can be added to queue...\n");
#endif
        for(int i=0 ; i<n ; i++){ // For every process...
            if (allProcesses[i].arrivalTime==time){ // If arrTime is reached...
#if DEBUG_MODE
printf("Proc %s has matching arrTime: %d\n", allProcesses[i].ID, allProcesses[i].arrivalTime);
#endif
                queue = realloc(queue, (queueLen+1)*sizeof(struct Process));
                if (!queue) {fprintf(stderr, "ERROR: realloc() failed\n"); return EXIT_FAILURE;}
                queue[queueLen] = allProcesses[i];
                queue[queueLen].state = 1; //state=IN-QUEUE
                queueLen += 1;
                
                printf("time %dms: Process %s arrived; added to ready queue",
                    time, queue[queueLen-1].ID);
                queue_status(queue, queueLen);
#if DEBUG_MODE
sleep(SLEEP_TIME);
#endif
            }
        }


#if DEBUG_MODE
printf("============================================================================\n");
#endif



    // Exit Condition
    if (completedProcs==n){break;}

    /*
    1. check context switch out start
    2. check context switch out done
    3. check context switch in start
    4. check context switch in done
    5. check IO
    6. check arrival
    */

    // Check all "next events" and increase time based on smallest amount
    int advance = 0;
    if (cpuIsRunning && cpuProc.cpuBurstCurr==0 && csLeft!=0) {advance = csLeft;} // If cs-out-of-CPU is smallest, adv by that
    if (queueLen!=0 && !cpuIsRunning && csLeft!=0) {if (csLeft<advance) {advance = csLeft;}} // If cs-into-CPU is smallest, adv by that
    if (cpuIsRunning && cpuProc.cpuBurstCurr!=0) {if (cpuProc.cpuBurstCurr<advance) {advance = cpuProc.cpuBurstCurr;}} // If curr CPU burst is smallest, adv by that
    if (ioLen!=0) {if (io->head->p.ioBurstCurr<advance) {advance = io->head->p.ioBurstCurr;}} // If curr I/O burst is smallest, adv by that
    int nextArr = 2147483647;
    for(int i=0 ; i<n ; i++){
        if (allProcesses[i].state==2 && allProcesses[i].arrivalTime>time && allProcesses[i].arrivalTime<nextArr) {nextArr = allProcesses[i].arrivalTime;}
    }
    if (nextArr<advance) {advance = nextArr;} // If next arrival time is smallest, adv by that

#if DEBUG_MODE
printf("Attempting to advance by %dms...\n", advance);
#endif

    // Advanc all time-keeping/-tracking variables 
    time+=advance; // Increase overall time variable
    if (cpuIsRunning && cpuProc.cpuBurstCurr!=0){cpuProc.cpuBurstCurr -= advance;} // "If CPU is occupied but burst time is done, decrease burst time"
    else if (cpuIsRunning && cpuProc.cpuBurstCurr==0) {csLeft -= advance;} // "If CPU is occupied but burst is done, decrease cs time"
    if (ioLen!=0) { // Deacrease all I/O burst times for all I/O processes
        struct ProcessPlus* curr = io->head;
        while(curr!=NULL){
            curr->p.ioBurstCurr -= advance;
            curr = curr->next;
        }
    }
}

    // simout.txt info
    // [...]

    return EXIT_SUCCESS;
}