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

#define SLEEP_TIME 1



// Struct for holding info for a single process
struct Process{
    char* ID; // Process ID (A0 - Z9)
    int state;   // States: IN-CPU (0) / IN-QUEUE (1) / IN-MEMORY (2) / IN-I/O (3) / TERMINATED (4)
    int binding;   // Binding: CPU-Bound (0) / I/O-Bound (1)
    
    int arrivalTime;   // Process arrival time
    int cpuBurstCount;   // Number of CPU burst for a single process
    // (Note: ioBurstCount = cpuBurstCount-1)
    int idx;   // Variable for indexing CPU bursts and I/O bursts
    
    // Containers for storing CPU and I/O burst times
    int* cpuBurstTimes;
    int* ioBurstTimes;

    int cpuBurstCurr;
    int ioBurstCurr;
};

// Struct for representing a node in a linked list
struct ProcessPlus{
    struct ProcessPlus* prev;
    struct ProcessPlus* next;
    struct Process p;
};

// Struct for representing a queue as a linked list
struct Queue{
    struct ProcessPlus* head;
};



// Print relevant process statistics - mostly for debugging
void print_proc(struct Process p){
    printf("  Process %s is ", p.ID);
    if (p.state==0) {printf("IN-CPU");}
    if (p.state==1) {printf("IN-QUEUE");}
    if (p.state==2) {printf("IN-MEMORY");}
    if (p.state==3) {printf("IN-I/O");}
    printf(" bound by ");
    if (p.binding==0) {printf("CPU\n");}
    if (p.binding==1) {printf("I/O\n");}
    printf("  Total CPU Bursts: %d  //", p.cpuBurstCount);
    printf("  Total I/O Bursts: %d\n", p.cpuBurstCount-1);
    printf("  Index: %d\n", p.idx);
    printf("  Current CPU Burst: %d  //", p.cpuBurstCurr);
    printf("  Current I/O Burst: %d\n", p.ioBurstCurr);
}

// Function for generating next pseudo-random number - can return either drand48() directly or an ouput of a function of lambda
float next_exp(float lambda, int bound, char* rounding, int only_drand){
    double r, x;
    while(1){
        r = drand48();   // uniform dist [0.00,1.00)

        if(only_drand){   // "Function ONLY asking for next value of just drand48()"
            if(r<=bound) {return r;}
        }

        x = (-log(r)/lambda);   // generate next pseudo-random value
        // (Note: log() = natural log)
        
        if(strcmp(rounding, "ceil")) {x = ceil(x);}
        if(strcmp(rounding, "floor")) {x = floor(x);}

        if(x<=bound) {break;}
    }

    return x;
}

// Generate IDs for processes
char** gen_IDs(int n){
    char** ret = (char**) malloc(n*sizeof(char*));
    if (ret==NULL) {fprintf(stderr, "ERROR: gen_IDs calloc() failed\n"); return NULL;}
    
    int i = 0;
    for (char let='A' ; let<='Z' ; let++) {   // Loop A-Z
        for (char num='0' ; num<='9' ; num++) {   // Loop 0-9
            ret[i] = (char*) malloc(3*sizeof(char));
            ret[i][0] = let; ret[i][1] = num; ret[i][2] = '\0';
            i++;
            if (i==n) {return ret;}
        }
    }
    return ret;
}

// Generate all processes to be used in an algorithm
struct Process* gen_procs(char** IDs, int seed, int n, int n_cpu, float lambda, int bound){
    srand48(seed);
    struct Process* allProcesses = calloc(n, sizeof(struct Process)); //DYNAMIC.MEMORY

    // Remember to free in some way: free(allProcesses)
    for (int i=0 ; i<n ; i++){
        int state = 2; // State 2 = waiting

        int binding; if(i<n_cpu) {binding=0;} else {binding=1;}

        float arrivalTime = next_exp(lambda, bound, "ceil", 0); // ON THE HANDOUT IT SHOULD SAY TO USE CEIL NOT FLOOR WTF
        
        int cpuBurstCount = ceil(next_exp(lambda, bound, "-", 1)*32);

        int idx = 0;
        
        int* cpuBurstTimes = calloc(cpuBurstCount, sizeof(int));
        int* ioBurstTimes = calloc(cpuBurstCount-1, sizeof(int));
        int cpuBurstTime, ioBurstTime;
        // For all same-index CPU and I/O bursts
        for (int i=0 ; i<cpuBurstCount-1 ; i++){
            if(binding==0){ //CPU-bound
                cpuBurstTime = ceil(next_exp(lambda, bound, "-", 0)*4);
                ioBurstTime = next_exp(lambda, bound, "ceil", 0)+1;
            }
            else if(binding==1){ //I/O-bound
                cpuBurstTime = next_exp(lambda, bound, "ceil", 0)+1;
                ioBurstTime = ceil(next_exp(lambda, bound, "-", 0)*8);
            }
            cpuBurstTimes[i] = cpuBurstTime; ioBurstTimes[i] = ioBurstTime;
        }
        // For the last CPU burst
        if(binding==0) {cpuBurstTime = ceil(next_exp(lambda, bound, "-", 0)*4);}
        else if(binding==1) {cpuBurstTime = next_exp(lambda, bound, "ceil", 0)+1;}
        cpuBurstTimes[cpuBurstCount-1] = cpuBurstTime;

        int cpuBurstCurr = 0;   // Current CPU burst to focus on
        int ioBurstCurr = 0;   // Current I/O burst to focus on

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
        
        struct Process proc = {IDs[i], state, binding, arrivalTime, cpuBurstCount, idx, cpuBurstTimes, ioBurstTimes, cpuBurstCurr, ioBurstCurr};
        allProcesses[i] = proc;
    }

    return allProcesses;
}

// Function for adding process to queue, sorted by shortest current I/O burst
void queue_push(struct Queue* q, struct Process p_in){
    struct ProcessPlus p2 = {NULL, NULL, p_in};
    struct ProcessPlus* node = malloc(sizeof(struct ProcessPlus));
    if (node == NULL) {fprintf(stderr, "ERROR: add() failed\n"); exit(EXIT_FAILURE);}

    *node = p2;
    node->prev = NULL;
    node->next = NULL;

    if (q->head==NULL) {q->head = node;}
    else{
        struct ProcessPlus* curr = q->head;
        if (curr->p.ioBurstCurr<=p_in.ioBurstCurr){
            curr->prev=node;
            curr->prev->next=curr;
            q->head=node;
        }else{
            while (curr->next != NULL) {
                curr=curr->next;
                if (curr->p.ioBurstCurr<=p_in.ioBurstCurr){
                    curr->prev=node;
                    curr->prev->next=curr;
                    break;
                }
            }
        }
    }
}

// Function for removing a process from a queue, which is sorted by shortest current I/O burst
struct Process queue_pop(struct Queue* q){
    struct ProcessPlus* curr = q->head;

    if (curr->next!=NULL) {curr->next->prev = NULL;}
    q->head = curr->next;

    struct Process ret = curr->p;
    free(curr);

    return ret;
}


void queue_status(struct Process* queue, int queueLen){
// #if DEBUG_MODE
// printf("\n...checking queue...\n");
// #endif
    printf(" [Q");
    if (queueLen==0) {printf(" empty");}
    else {
        for(int i=0 ; i<queueLen ; i++) {printf(" %s", queue[i].ID);}}
    printf("]\n");
}



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


// void SJF(struct Process* allProcesses, int n){
//     int time = 0;
// }


// void SRT(){
//     int time = 0;
// }


// void RR(){
//     int time = 0;

// }