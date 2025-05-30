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



int RR(struct Process* allProcesses, int n, int t_cs, int t_slice, int fd, ssize_t bytesWritten, char toWrite[200]){
    int time = 0;
    #if DEBUG_MODE
    int timeAdd = 0;
    #endif
    int csLeft = t_cs/2;
    int tsliceLeft = t_slice;
    
    // Priority Queue = array of Process pointers
    struct Process** priorityQueue = calloc(0, sizeof(struct Process)); //DYNAMIC.MEMORY
    if (priorityQueue==NULL) {fprintf(stderr, "ERROR: calloc() failed\n"); exit(EXIT_FAILURE);}
    // Remember to free in some way: free(priorityQueue)
    int priorityQueueLen = 0;

    // Pointer to "in-CPU" process
    struct Process* cpuProc;
    int cpuIsRunning = 0;   // Boolean to see if a process is in CPU
    int preemptFlag = 0;    // Boolean to see if a process needs to be removed from CPU
    
    // I/O = Sorted Linked List / Queue struct of Process pointers
    struct Queue* io = calloc(1, sizeof(struct Queue)); //DYNAMIC.MEMORY
    if (io==NULL) {fprintf(stderr, "ERROR: malloc() failed\n"); exit(EXIT_FAILURE);}
    // Remember to free in some way: free(io)
    int ioLen = 0;

    int completedProcs = 0;



    //======================================================================================================================
    #if DEBUG_MODE
    printf("~~ ===BEGINNING RR===\n");
    #endif
    printf("time %dms: Simulator started for RR", time);
    priority_queue_status(priorityQueue, priorityQueueLen);
    while(1){
        #if DEBUG_MODE
        printf("~~ TIME: %d\n", time);
        #endif


        //======================================================================================================================
        #if DEBUG_MODE
        printf("~~ Checking if current process is set to be preempted...\n");
        #endif
        if (preemptFlag){ // If preempt has been triggered, treat the current CPU process as if it had been completed
            if(csLeft==0 && cpuProc->state==4){ // If done cs'ing...
                csLeft = t_cs/2; //Reset
                tsliceLeft = t_slice; //Reset
                cpuIsRunning = 0; // "CPU is no longer running a process"

                priorityQueue = realloc(priorityQueue, (priorityQueueLen+1)*sizeof(struct Process*));
                if (!priorityQueue) {fprintf(stderr, "ERROR: realloc() failed\n"); return EXIT_FAILURE;}
                priorityQueue[priorityQueueLen] = cpuPop(cpuProc); // Remove curr-proc, add to back of queue
                priorityQueue[priorityQueueLen]->state = 1; //state=IN-QUEUE
                priorityQueueLen += 1;
                preemptFlag = 0; //Reset
            }else if (cpuProc->state!=4){ // Change state of curr process to POST-CPU
                cpuProc->state = 4;
                cpuProc->preemptState = 1;  // Indicate current CPU burst was preempted
                if (time<10000){
                    printf("time %dms: Time slice expired; preempting process %s with %dms remaining", time, cpuProc->ID, cpuProc->cpuBurstCurr);
                    priority_queue_status(priorityQueue, priorityQueueLen);
                }
            }
        }else if (cpuIsRunning && cpuProc->cpuBurstCurr!=0 && priorityQueueLen==0){
            if(tsliceLeft==0 && !preemptFlag){
                tsliceLeft = t_slice; //Reset
                if (time<10000){
                    printf("time %dms: Time slice expired; no preemption because ready queue is empty", time);
                    priority_queue_status(priorityQueue, priorityQueueLen);
                }
            }
        }


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
                tsliceLeft = t_slice; //Reset
                cpuIsRunning = 0; // "CPU no longer running a proccess"
                cpuProc->preemptState = 0; //Reset

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
                        printf("time %dms: Process %s switching out of CPU; blocking on I/O until time %dms",
                            time, cpuProc->ID, (time+(t_cs/2)+cpuProc->ioBurstCurr));
                        priority_queue_status(priorityQueue, priorityQueueLen);
                    }
                }
            #if DEBUG_MODE
            sleep(SLEEP_TIME_EVENT+timeAdd);
            #endif
            }
        }


        //======================================================================================================================
        #if DEBUG_MODE
        printf("~~ Checking if process can be added to CPU or Pre-CPU stage...\n");
        #endif
        if (priorityQueueLen!=0 && !cpuIsRunning){ // If queue isn't empty and nothing is in the CPU... 
            if (csLeft==0){  // If done cs'ing...
                csLeft = t_cs/2; //Reset
                tsliceLeft = t_slice; //Reset
                cpuProc = pop(priorityQueue, priorityQueueLen); // Get latest-in-queue process; this also resizes queue
                if (priorityQueue==NULL) {fprintf(stderr, "ERROR: realloc() failed\n"); return EXIT_FAILURE;}
                priorityQueueLen -= 1;
                cpuProc->state = 3; //state==IN-CPU
                if (cpuProc->preemptState==0) {cpuProc->cpuBurstCurr = cpuProc->cpuBurstTimes[cpuProc->idx];} // Establish current CPU burst time
                cpuIsRunning = 1;   // "CPU is now running a proccess"

                if (time<10000){
                    if (cpuProc->preemptState==0) {
                        printf("time %dms: Process %s started using the CPU for %dms burst",
                            time, cpuProc->ID, cpuProc->cpuBurstCurr);
                    }else{
                        printf("time %dms: Process %s started using the CPU for remaining %dms of %dms burst",
                            time, cpuProc->ID, cpuProc->cpuBurstCurr, cpuProc->cpuBurstTimes[cpuProc->idx]);
                    }
                    priority_queue_status(priorityQueue, priorityQueueLen);
                }
            #if DEBUG_MODE
                sleep(SLEEP_TIME_EVENT+timeAdd);
                #endif
            }else{
                priorityQueue[0]->state = 2; //state==PRE-CPU
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

                    if (time<10000){
                        printf("time %dms: Process %s completed I/O; added to ready queue",
                            time, priorityQueue[priorityQueueLen-1]->ID);
                        priority_queue_status(priorityQueue, priorityQueueLen);
                    }
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
                
                if (time<10000){
                    printf("time %dms: Process %s arrived; added to ready queue",
                        time, priorityQueue[priorityQueueLen-1]->ID);
                    priority_queue_status(priorityQueue, priorityQueueLen);
                }
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
            printf("time %dms: Simulator ended for RR", time);
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

        if (preemptFlag && csLeft!=0){ // If preempt cs-out-of-CPU is smallest, adv by that
            advance = csLeft;
            #if DEBUG_MODE
            printf("~~ Found new advancing time: preempt cs-out-of-CPU time of %d\n", advance);
            #endif
        }
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
        if (tsliceLeft<advance && cpuProc){ // If time slice is smallest, adv by that
            if (cpuProc->state!=4){advance = tsliceLeft;}
            #if DEBUG_MODE
            printf("~~ Found new advancing time: time slice time of %d\n", advance);
            #endif
        }
        // Go through processes in CPU queue and add wait time
        for (int i = 0; i<priorityQueueLen; i++){priorityQueue[i]->cpuWaitTime += advance;}
        // Advance turnaround time if process is in states 1-4
        for (int i=0; i<n; i++){
            if (allProcesses[i].state==1 || allProcesses[i].state==2 || allProcesses[i].state==3 || allProcesses[i].state==4){
                allProcesses[i].cpuTurnAround += advance;
            }
        }


        //======================================================================================================================
        #if DEBUG_MODE
        printf("~~ Attempting to advance by %dms...\n", advance);
        #endif
        time+=advance; // Increase overall time variable
        
        if (cpuIsRunning && cpuProc->cpuBurstCurr!=0){ // "If CPU is occupied but burst time isn't done, decrease burst time"
            cpuProc->cpuBurstCurr -= advance;
            tsliceLeft -= advance;
            if (preemptFlag) {csLeft -= advance; cpuProc->cpuBurstCurr += advance;}
            // If CPU isn't done within time slice and there are processes in the ready queue, set preempt flag and increment preempt count
            if(cpuProc->cpuBurstCurr!=0 && tsliceLeft==0 && priorityQueueLen!=0){preemptFlag = 1; cpuProc->preempts++;}
            #if DEBUG_MODE
            printf("~~ CPU running, CPU burst not done: decreasing CPU burst time\n");
            #endif
        }
        else if (cpuIsRunning && cpuProc->cpuBurstCurr==0){ // "If CPU is occupied but burst is done, decrease cs time"
            csLeft -= advance;
            // If CPU finishes within time slice, increment withinSlice count and reset
            if(tsliceLeft>=0){
                if (cpuProc->preemptState==0){cpuProc->withinSlice++;}
                tsliceLeft = t_slice;
            }
            #if DEBUG_MODE
            printf("~~ CPU running, CPU burst done: decreasing cs time\n");
            #endif
        }
        else if (priorityQueueLen!=0 && !cpuIsRunning){ // "If CPU isn't occupied but cs isn't over, decrease cs time"
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
        sleep(SLEEP_TIME_ADVANCING);
        //if(time>2000) {timeAdd = 3;}
        #endif
    }


    //======================================================================================================================
    #if DEBUG_MODE
    printf("~~ Printing simout.txt information...\n");
    #endif
    sprintf(toWrite, "\nAlgorithm RR\n");
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    double cpuUtil = 0;
    int cpuTotalBursts = 0;
    int ioTotalBursts = 0;
    double cpuTotalWaitTime = 0;
    double ioTotalWaitTime = 0;
    double cpuTotalTATime = 0;
    double ioTotalTATime = 0;
    int numCPUcs = 0;
    int numIOcs = 0;
    int cpuTotalPreempts = 0;
    int ioTotalPreempts = 0;
    double cpuTotalWithin = 0;
    double ioTotalWithin = 0;
    for(int i=0 ; i<n ; i++){ // For every process...
        for(int j=0 ; j<allProcesses[i].cpuBurstCount ; j++){ // For every CPU burst time
            cpuUtil += allProcesses[i].cpuBurstTimes[j];
        }
        
        if (allProcesses[i].binding==0) { // If process is CPU bound...
            numCPUcs += allProcesses[i].cpuBurstCount; // # context switches = cpu burst count + total cpu preempts
            cpuTotalWaitTime += allProcesses[i].cpuWaitTime; // Add to total wait time (with context switches)
            cpuTotalWaitTime -= allProcesses[i].cpuBurstCount*(t_cs/2); // Remove context switches from total wait time
            cpuTotalTATime += allProcesses[i].cpuTurnAround; // Add to total turnaround time
            cpuTotalBursts += allProcesses[i].cpuBurstCount;
            cpuTotalPreempts += allProcesses[i].preempts;
            numCPUcs += allProcesses[i].preempts;
            cpuTotalWaitTime -= allProcesses[i].preempts*(t_cs/2);
            cpuTotalWithin += allProcesses[i].withinSlice;
        }else{ // If process is I/O bound...
            numIOcs += allProcesses[i].cpuBurstCount; // # context switches = cpu burst count + total io preempts
            ioTotalWaitTime += allProcesses[i].cpuWaitTime; // Total wait time (with context switches)
            ioTotalWaitTime -= allProcesses[i].cpuBurstCount*(t_cs/2); // Remove context switches from total wait time
            ioTotalTATime += allProcesses[i].cpuTurnAround; // Add to total turnaround time
            ioTotalBursts += allProcesses[i].cpuBurstCount;
            ioTotalPreempts += allProcesses[i].preempts;
            numIOcs += allProcesses[i].preempts;
            ioTotalWaitTime -= allProcesses[i].preempts*(t_cs/2);
            ioTotalWithin += allProcesses[i].withinSlice;
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
    // Preemptions
    sprintf(toWrite, "-- CPU-bound number of preemptions: %d\n", cpuTotalPreempts);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- I/O-bound number of preemptions: %d\n", ioTotalPreempts);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- overall number of preemptions: %d\n", cpuTotalPreempts+ioTotalPreempts);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    // RR specific stats
    sprintf(toWrite, "-- CPU-bound percentage of CPU bursts completed within one time slice: %.3f%%\n", ceil((cpuTotalWithin/cpuTotalBursts)*100000)/1000);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- I/O-bound percentage of CPU bursts completed within one time slice: %.3f%%\n", ceil((ioTotalWithin/ioTotalBursts)*100000)/1000);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}
    sprintf(toWrite, "-- overall percentage of CPU bursts completed within one time slice: %.3f%%\n", 
        ceil(((cpuTotalWithin+ioTotalWithin)/(cpuTotalBursts+ioTotalBursts))*100000)/1000);
    bytesWritten = write(fd, toWrite, strlen(toWrite));
    if (bytesWritten==-1) {fprintf(stderr, "ERROR: write() failed\n"); close(fd); return EXIT_FAILURE;}


    //======================================================================================================================
    free(priorityQueue);
    free(io);

    return EXIT_SUCCESS;
}
