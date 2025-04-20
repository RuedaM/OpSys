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

// #if DEBUG_MODE
// #define SLEEP_TIME_ADVANCING 0
// #define SLEEP_TIME_EVENT 0
// #endif



int SJFnoTau(struct Process* allProcesses, int n, int t_cs, int fd, ssize_t bytesWritten, char toWrite[200]){
    int time = 0;
    // #if DEBUG_MODE
    // int timeAdd = 0;
    // #endif
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



    //======================================================================================================================
    #if DEBUG_MODE
    printf("~~ ===BEGINNING SJF===\n");
    #endif
    printf("time %dms: Simulator started for SJF", time);
    priority_queue_status(priorityQueue, priorityQueueLen);
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

            if(csLeft==0 && cpuProc->state==4){ // If done cs'ing...
                csLeft = t_cs/2; //Reset
                cpuIsRunning = 0; // "CPU no longer running a proccess"

                if ((cpuProc->idx+1)==cpuProc->cpuBurstCount){ // If CPUProc has no more CPU bursts...
                    #if DEBUG_MODE
                    printf("~~ End of process...\n");
                    #endif
                    cpuProc->state = 6; //state=TERMINATED
                    completedProcs += 1;
                }else{ // If CPUProc still has CPU bursts...
                    #if DEBUG_MODE
                    printf("~~ Into I/O...\n");
                    #endif
                    cpuProc->state = 5; //state==IN-I/O
                    cpuProc->ioBurstCurr = cpuProc->ioBurstTimes[cpuProc->idx]; // Establish current I/O burst time
                    queue_push_to_end(io, cpuProc);
                    ioLen += 1;
                }
            }else if (cpuProc->state!=4){ // If not, update states and print info
                cpuProc->state = 4; //state==POST-CPU
                
                if ((cpuProc->idx+1)==cpuProc->cpuBurstCount){ // If CPUProc has no more CPU bursts...
                    printf("time %dms: Process %s terminated", time, cpuProc->ID);
                    priority_queue_status(priorityQueue, priorityQueueLen);
                }else{ // If CPUProc still has CPU bursts...
                    if (time<10000){
                        printf("time %dms: Process %s completed a CPU burst; %d burst",
                            time, cpuProc->ID, (cpuProc->cpuBurstCount-(cpuProc->idx+1)));
                        if (cpuProc->cpuBurstCount-(cpuProc->idx+1) == 1) {printf(" to go");}
                        else                                              {printf("s to go");}
                        priority_queue_status(priorityQueue, priorityQueueLen);
                    }

                    if (time<10000){
                        printf("time %dms: Process %s switching out of CPU; blocking on I/O until time %dms",
                            time, cpuProc->ID, (time+(t_cs/2)+cpuProc->ioBurstCurr));
                        priority_queue_status(priorityQueue, priorityQueueLen);
                    }
                }
            // #if DEBUG_MODE
            // sleep(SLEEP_TIME_EVENT+timeAdd);
            // #endif
            }
        }


        //======================================================================================================================
        #if DEBUG_MODE
        printf("~~ Checking if process can be added to CPU or Pre-CPU stage...\n");
        #endif
        if (priorityQueueLen!=0 && !cpuIsRunning){ // If queue isn't empty and nothing is in the CPU...
            if (csLeft==0){  // If done cs'ing...
                csLeft = t_cs/2; //Reset

                cpuProc = pop(priorityQueue, priorityQueueLen); // Get latest-in-queue process; this also resizes queue
                if (priorityQueue==NULL) {fprintf(stderr, "ERROR: realloc() failed\n"); return EXIT_FAILURE;}
                priorityQueueLen -= 1;
                cpuProc->state = 3; //state==IN-CPU
                cpuIsRunning = 1;   // "CPU is now running a proccess"

                if (time<10000){
                    printf("time %dms: Process %s started using the CPU for %dms burst",
                        time, cpuProc->ID, cpuProc->cpuBurstCurr);
                    priority_queue_status(priorityQueue, priorityQueueLen);
                }

                // #if DEBUG_MODE
                // sleep(SLEEP_TIME_EVENT+timeAdd);
                // #endif
            }else{
                if (io->head!=NULL && io->head->p->ioBurstCurr==0
                && io->head->p->cpuBurstTimes[io->head->p->idx+1]<priorityQueue[0]->cpuBurstTimes[priorityQueue[0]->idx+1]){
                    io->head->p->state = 2; //state==PRE-CPU
                }else{
                    priorityQueue[0]->state = 2; //state==PRE-CPU
                }
            }
        }


        //======================================================================================================================
        #if DEBUG_MODE
        printf("~~ Checking if process can be added from I/O back to queue...\n");
        #endif
        while(1){ // Until all 0-I/O-burst-time processes have been moved...
            
            if (ioLen!=0){ // If I/O isn't empty...
                if (io->head->p->ioBurstCurr==0){ // If next-up-proc's I/O burst is finished...
                    
                    struct Process* movingProc = queue_pop(io); // Remove next-up-proc, add to back of queue
                    movingProc->state = 1; //state=IN-QUEUE
                    movingProc->idx += 1; //Increase CPU and I/O bust times index
                    movingProc->cpuBurstCurr = movingProc->cpuBurstTimes[movingProc->idx]; // Establish current CPU burst time
                    priority_queue_push_SJFnoTau(&priorityQueue, priorityQueueLen, movingProc);
                    priorityQueueLen += 1;
                    ioLen -= 1;

                    if (time<10000){
                        printf("time %dms: Process %s completed I/O; added to ready queue",
                            time, movingProc->ID);
                        priority_queue_status(priorityQueue, priorityQueueLen);
                    }
                }else {break;} // If next-up-proc's I/O burst isn't finished, continue past
            }else{break;} // If I/O is empty, continue past
            // #if DEBUG_MODE
            // sleep(SLEEP_TIME_EVENT+timeAdd);
            // #endif
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
                struct Process* movingProc = &allProcesses[i];
                movingProc->state = 1; //state=IN-QUEUE
                movingProc->cpuBurstCurr = movingProc->cpuBurstTimes[movingProc->idx]; // Establish current CPU burst time
                priority_queue_push_SJFnoTau(&priorityQueue, priorityQueueLen, movingProc);
                priorityQueueLen += 1;
                
                if (time<10000){
                    printf("time %dms: Process %s arrived; added to ready queue",
                        time, movingProc->ID);
                    priority_queue_status(priorityQueue, priorityQueueLen);
                }
                // #if DEBUG_MODE
                // sleep(SLEEP_TIME_EVENT+timeAdd);
                // #endif
            }
        }



        //======================================================================================================================
        #if DEBUG_MODE
        printf("~~ Checking if algo is done...\n");
        #endif        
        if (completedProcs==n){
            printf("time %dms: Simulator ended for SJF", time);
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
            if (allProcesses[i].state==0 && allProcesses[i].arrivalTime>time){
                if(allProcesses[i].arrivalTime<nextArr){
                    nextArr = allProcesses[i].arrivalTime;
                }
            }
        }
        if ((nextArr-time)<advance){ // If next arrival time is smallest, adv by that
            advance = nextArr-time;
            #if DEBUG_MODE
            printf("~~ Found new advancing time: next arrival time of %d\n", advance);
            #endif
        }


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
        printf("~~ queueLen = %d\n", priorityQueueLen);
        printf("~~ ioLen = %d\n", ioLen);
        printf("~~ ============================================================================\n");
        // sleep(SLEEP_TIME_ADVANCING);
        //if(time>2000) {timeAdd = 3;}
        #endif
    }


    //======================================================================================================================
    #if DEBUG_MODE
    printf("~~ Printing simout.txt information...\n");
    #endif
    sprintf(toWrite, "\nAlgorithm SJF\n");
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    float cpuUtil = 0;
    int cpuTotalBursts = 0;
    int ioTotalBursts = 0;
    float cpuTotalWaitTime = 0;
    float ioTotalWaitTime = 0;
    float cpuTotalTATime = 0;
    float ioTotalTATime = 0;
    int numCPUcs = 0;
    int numIOcs = 0;
    for(int i=0 ; i<n ; i++){ // For every process...
        for(int j=0 ; j<allProcesses[i].cpuBurstCount ; j++){ // For every CPU burst time...
            cpuUtil += allProcesses[i].cpuBurstTimes[j]; // Add all CPU burst times
        }
        
        if (allProcesses[i].binding==0) { // If process is CPU-bound...
            numCPUcs += allProcesses[i].cpuBurstCount; // # context switches = cpu burst count
            cpuTotalWaitTime += allProcesses[i].cpuWaitTime; // Add to total wait time (with context switches)
            cpuTotalWaitTime -= allProcesses[i].cpuBurstCount*(t_cs/2); // Remove context switches from total wait time
            cpuTotalTATime += allProcesses[i].cpuTurnAround; // Add to total turnaround time
            cpuTotalBursts += allProcesses[i].cpuBurstCount;
        }else{ // If process is I/O-bound...
            numIOcs += allProcesses[i].cpuBurstCount; // # context switches = cpu burst count
            ioTotalWaitTime += allProcesses[i].cpuWaitTime; // Total wait time (with context switches)
            ioTotalWaitTime -= allProcesses[i].cpuBurstCount*(t_cs/2); // Remove context switches from total wait time
            ioTotalTATime += allProcesses[i].cpuTurnAround; // Add to total turnaround time
            ioTotalBursts += allProcesses[i].cpuBurstCount;
        }
    }
    cpuUtil /= time;

    // CPU utilization
    sprintf(toWrite, "-- CPU utilization: %.3f%%\n", ceil(cpuUtil*100000)/1000);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    // Wait times
    sprintf(toWrite, "-- CPU-bound average wait time: %.3f ms\n", ceil((cpuTotalWaitTime/cpuTotalBursts)*1000)/1000);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- I/O-bound average wait time: %.3f ms\n", ceil((ioTotalWaitTime/ioTotalBursts)*1000)/1000);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- overall average wait time: %.3f ms\n", ceil(((cpuTotalWaitTime+ioTotalWaitTime)/(cpuTotalBursts+ioTotalBursts))*1000)/1000);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    // Turnaround times
    sprintf(toWrite, "-- CPU-bound average turnaround time: %.3f ms\n", ceil((cpuTotalTATime/cpuTotalBursts)*1000)/1000);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- I/O-bound average turnaround time: %.3f ms\n", ceil((ioTotalTATime/ioTotalBursts)*1000)/1000);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- overall average turnaround time: %.3f ms\n", ceil(((cpuTotalTATime+ioTotalTATime)/(cpuTotalBursts+ioTotalBursts))*1000)/1000);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    // Context switches
    sprintf(toWrite, "-- CPU-bound number of context switches: %d\n", numCPUcs);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- I/O-bound number of context switches: %d\n", numIOcs);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- overall number of context switches: %d\n", numCPUcs+numIOcs);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    // FCFS has no preemptions
    sprintf(toWrite, "-- CPU-bound number of preemptions: 0\n");
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- I/O-bound number of preemptions: 0\n");
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- overall number of preemptions: 0\n");
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}



    //======================================================================================================================
    free(priorityQueue);
    free(io);

    return EXIT_SUCCESS;
}
