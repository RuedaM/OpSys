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

#if DEBUG_MODE
#define SLEEP_TIME_ADVANCING 0
#define SLEEP_TIME_EVENT 0
#endif



int FCFS(struct Process* allProcesses, int n, int t_cs){
    int time = 0;
    #if DEBUG_MODE
    int timeAdd = 0;
    #endif
    int csLeft = t_cs/2;
    
    // Priority Queue = array of Process pointers
    struct Process** priorityQueue = calloc(0, sizeof(struct Process)); //DYNAMIC.MEMORY
    if (priorityQueue==NULL) {fprintf(stderr, "ERROR: calloc() failed\n"); exit(EXIT_FAILURE);}
    // Remember to free in some way: free(priorityQueue)
    int priorityQueueLen = 0;

    // Pointer to "in-CPU" process
    struct Process* cpuProc;
    int cpuIsRunning = 0;   // Boolean to see if a process is in CPU
    
    // I/O = Sorted Linked List / Queue struct of Process pointers
    struct Queue* io = calloc(1, sizeof(struct Queue)); //DYNAMIC.MEMORY
    if (io==NULL) {fprintf(stderr, "ERROR: malloc() failed\n"); exit(EXIT_FAILURE);}
    // Remember to free in some way: free(io)
    int ioLen = 0;

    int completedProcs = 0;

    printf("time %dms: Simulator started for FCFS", time);
    priority_queue_status(priorityQueue, priorityQueueLen);



    //======================================================================================================================
    #if DEBUG_MODE
    printf("~~ ===BEGINNING FCFS===\n");
    #endif
    while(1){
        #if DEBUG_MODE
        printf("~~ TIME: %d\n", time);
        #endif


        //======================================================================================================================
        #if DEBUG_MODE
        printf("~~ Checking if process can be deleted or added to I/O...\n");
        #endif
        if (cpuIsRunning && cpuProc->cpuBurstCurr==0){ // If CPU in-use and cpuProc's burst is done...
            
            if (cpuProc->idx!=cpuProc->cpuBurstCount-1){ // If proc not about to terminate, set current I/O burst time
                cpuProc->ioBurstCurr = cpuProc->ioBurstTimes[cpuProc->idx];
            }

            if(csLeft==0){ // If done cs'ing...
                csLeft = t_cs/2; //Reset
                cpuIsRunning = 0; // "CPU no longer running a proccess"

                if ((cpuProc->idx+1)==cpuProc->cpuBurstCount){ // If CPUProc has no more CPU bursts...
                    #if DEBUG_MODE
                    printf("~~ End of process...\n");
                    #endif
                    cpuProc->state = 4; //state=TERMINATED
                    completedProcs += 1;
                }else{ // If CPUProc still has CPU bursts...
                    #if DEBUG_MODE
                    printf("~~ Into I/O...\n");
                    #endif
                    cpuProc->state = 3; //state==IN-I/O
                    cpuProc->ioBurstCurr = cpuProc->ioBurstTimes[cpuProc->idx];   // Establish current I/O burst time
                    queue_push_to_end(io, cpuProc);
                    ioLen += 1;
                }
            }else{ // If not, just print info
                if ((cpuProc->idx+1)==cpuProc->cpuBurstCount){ // If CPUProc has no more CPU bursts...
                    printf("time %dms: Process %s terminated", time, cpuProc->ID);
                    priority_queue_status(priorityQueue, priorityQueueLen);
                }else{ // If CPUProc still has CPU bursts...
                    printf("time %dms: Process %s completed a CPU burst; %d burst",
                        time, cpuProc->ID, (cpuProc->cpuBurstCount-(cpuProc->idx+1)));
                    if (cpuProc->cpuBurstCount-(cpuProc->idx+1) == 1) {printf(" to go");}
                    else                                              {printf("s to go");}
                    priority_queue_status(priorityQueue, priorityQueueLen);
                    printf("time %dms: Process %s switching out of CPU; blocking on I/O until time %dms",
                        time, cpuProc->ID, (time+(t_cs/2)+cpuProc->ioBurstCurr));
                    priority_queue_status(priorityQueue, priorityQueueLen);
                }
            #if DEBUG_MODE
            sleep(SLEEP_TIME_EVENT+timeAdd);
            #endif
            }
        }


        //======================================================================================================================
        #if DEBUG_MODE
        printf("~~ Checking if process can be added to CPU...\n");
        #endif
        if (priorityQueueLen!=0 && !cpuIsRunning){ // If queue isn't empty and nothing is in the CPU... 
            if (csLeft==0){  // If done cs'ing...
                csLeft = t_cs/2; //Reset
                cpuProc = pop(priorityQueue, priorityQueueLen); // Get latest-in-queue process; this also resizes queue
                if (priorityQueue==NULL) {fprintf(stderr, "ERROR: realloc() failed\n"); return EXIT_FAILURE;}
                priorityQueueLen -= 1;
                cpuProc->state = 0; //state==IN-CPU
                cpuProc->cpuBurstCurr = cpuProc->cpuBurstTimes[cpuProc->idx]; // Establish current CPU burst time
                cpuIsRunning = 1;   // "CPU is now running a proccess"

                printf("time %dms: Process %s started using the CPU for %dms burst",
                    time, cpuProc->ID, cpuProc->cpuBurstCurr);
                priority_queue_status(priorityQueue, priorityQueueLen);
                #if DEBUG_MODE
                sleep(SLEEP_TIME_EVENT+timeAdd);
                #endif
            }
        }


        //======================================================================================================================
        #if DEBUG_MODE
        printf("~~ Checking if process can be added back to queue after I/O...\n");
        #endif
        while(1){ // Until all 0-I/O-burst-time processes have been moved...
            
            if (ioLen!=0){ // If I/O isn't empty...
                if (io->head->p->ioBurstCurr==0){ // If next-up-proc's I/O burst is finished...
                    priorityQueue = realloc(priorityQueue, (priorityQueueLen+1)*sizeof(struct Process*));
                    if (!priorityQueue) {fprintf(stderr, "ERROR: realloc() failed\n"); return EXIT_FAILURE;}
                    
                    priorityQueue[priorityQueueLen] = queue_pop(io); // Remove next-up-proc, add to back of queue
                    priorityQueue[priorityQueueLen]->state = 1; //state=IN-QUEUE
                    priorityQueue[priorityQueueLen]->idx += 1; //Increase CPU and I/O bust times index
                    priorityQueueLen += 1;
                    ioLen -= 1;

                    printf("time %dms: Process %s completed I/O; added to ready queue",
                        time, priorityQueue[priorityQueueLen-1]->ID);
                    priority_queue_status(priorityQueue, priorityQueueLen);
                }else {break;} // If next-up-proc's I/O burst isn't finished, continue past
            }else{break;} // If I/O is empty, continue past
            #if DEBUG_MODE
            sleep(SLEEP_TIME_EVENT+timeAdd);
            #endif
        }


        //======================================================================================================================
        #if DEBUG_MODE
        printf("~~ Checking if arriving process in memory can be added to queue...\n");
        #endif
        for(int i=0 ; i<n ; i++){ // For every process...
            if (allProcesses[i].arrivalTime==time){ // If arrTime is reached...
                #if DEBUG_MODE
                printf("~~ Proc %s has matching arrTime: %d\n", allProcesses[i].ID, allProcesses[i].arrivalTime);
                #endif
                priorityQueue = realloc(priorityQueue, (priorityQueueLen+1)*sizeof(struct Process*));
                if (!priorityQueue) {fprintf(stderr, "ERROR: realloc() failed\n"); return EXIT_FAILURE;}
                priorityQueue[priorityQueueLen] = &allProcesses[i];
                priorityQueue[priorityQueueLen]->state = 1; //state=IN-QUEUE
                priorityQueueLen += 1;
                
                printf("time %dms: Process %s arrived; added to ready queue",
                    time, priorityQueue[priorityQueueLen-1]->ID);
                priority_queue_status(priorityQueue, priorityQueueLen);
                #if DEBUG_MODE
                sleep(SLEEP_TIME_EVENT+timeAdd);
                #endif
            }
        }

        //======================================================================================================================
        #if DEBUG_MODE
        printf("~~ Checking if algo is done...\n");
        #endif        
        if (completedProcs==n){
            printf("time %dms: Simulator ended for FCFS", time);
            priority_queue_status(priorityQueue, priorityQueueLen);
            printf("\n");
            break;
        }


        //======================================================================================================================
        #if DEBUG_MODE
        printf("~~ Updating all times...\n");
        #endif
        int advance = 2147483647; //~inf
        int nextArr;

        if (cpuIsRunning && cpuProc->cpuBurstCurr==0 && csLeft!=0){ // If cs-out-of-CPU is smallest, adv by that
            advance = csLeft;
            #if DEBUG_MODE
            printf("~~ Found new advancing time: cs-out-of-CPU time of %d\n", advance);
            #endif
        }
        if (priorityQueueLen!=0 && !cpuIsRunning && csLeft!=0){ // If cs-into-CPU is smallest, adv by that
            if (csLeft<advance){
                advance = csLeft;
                #if DEBUG_MODE
                printf("~~ Found new advancing time: cs-into-CPU time of %d\n", advance);
                #endif
            }
        }
        if (cpuIsRunning && cpuProc->cpuBurstCurr!=0){ // If curr CPU burst is smallest, adv by that
            if (cpuProc->cpuBurstCurr<advance){
                advance = cpuProc->cpuBurstCurr;
                #if DEBUG_MODE
                printf("~~ Found new advancing time: remaining CPU burst time of %d\n", advance);
                #endif
            }
        }
        nextArr = 2147483647;
        if (ioLen!=0){ // If curr I/O burst is smallest, adv by that
            if (io->head->p->ioBurstCurr<advance){
                advance = io->head->p->ioBurstCurr;
                #if DEBUG_MODE
                printf("~~ Found new advancing time: remining I/O burst time of %d\n", advance);
                #endif
            }
        }
        nextArr = 2147483647;
        for(int i=0 ; i<n ; i++){
            if (allProcesses[i].state==2 && allProcesses[i].arrivalTime>time && allProcesses[i].arrivalTime<nextArr) {nextArr = allProcesses[i].arrivalTime-time;}
        }
        if (nextArr<advance) {
            advance = nextArr;
            #if DEBUG_MODE
            printf("~~ Found new advancing time: next arrival time of %d\n", advance);
            #endif
        } // If next arrival time is smallest, adv by that


        //======================================================================================================================
        #if DEBUG_MODE
        printf("~~ Attempting to advance by %dms...\n", advance);
        #endif
        time+=advance; // Increase overall time variable
        
        if (cpuIsRunning && cpuProc->cpuBurstCurr!=0){ // "If CPU is occupied but burst time isn't done, decrease burst time"
            cpuProc->cpuBurstCurr -= advance;
            #if DEBUG_MODE
            printf("~~ CPU running, CPU burst not done: decreasing CPU burst time\n");
            #endif
        }
        else if (cpuIsRunning && cpuProc->cpuBurstCurr==0){ // "If CPU is occupied but burst is done, decrease cs time"
            csLeft -= advance;
            #if DEBUG_MODE
            printf("~~ CPU running, CPU burst done: decreasing cs time\n");
            #endif
        }
        else if (priorityQueueLen!=0 && !cpuIsRunning && csLeft!=0){ // "If CPU isn't occupied but cs isn't over, decrease cs time"
            csLeft -= advance;
            #if DEBUG_MODE
            printf("~~ Procs in-queue, CPU not running, cs time isn't done: decreasing cs time\n");
            #endif
        }
        
        if (ioLen!=0){ // Deacrease all I/O burst times for all I/O processes
            struct ProcessPlus* curr = io->head;
            while(curr!=NULL){
                curr->p->ioBurstCurr -= advance;
                curr = curr->next;
            }
            #if DEBUG_MODE
            printf("~~ Procs in I/O: decreasing all I/O burst times\n");
            #endif
        }
        #if DEBUG_MODE
        printf("~~ ============================================================================\n");
        print_all_proc(allProcesses, n);
        io_status(io);
        printf("~~ CPU running: %d\n", cpuIsRunning);
        printf("~~ queueLen = %d\n", queueLen);
        printf("~~ ioLen = %d\n", ioLen);
        printf("~~ ============================================================================\n");
        sleep(SLEEP_TIME_ADVANCING);
        //if(time>2000) {timeAdd = 3;}
        #endif
    }


    //======================================================================================================================
    #if DEBUG_MODE
    printf("~~ Printing simout.txt information...\n");
    #endif
    // [...]


    //======================================================================================================================
    free(priorityQueue);
    free(io);

    return EXIT_SUCCESS;
}
