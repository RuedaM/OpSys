#ifndef PROJECT_PART1_H
#define PROJECT_PART1_H

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

#define SLEEP_TIME 1


// =========================================================================================
// ======================================== STRUCTS ========================================
// =========================================================================================
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



// =========================================================================================
// ================================ .C FILE HELPER FUNCTIONS ===============================
// =========================================================================================
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



// =========================================================================================
// ============================== I/O "QUEUE" HELPER FUNCTIONS =============================
// =========================================================================================
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



// =========================================================================================
// ================================= FCFS HELPER FUNCTIONS =================================
// =========================================================================================
// Function for getting+removing 1st process in queue. Takes in queue array + its size
struct Process pop(struct Process* procQ, int numProc){
    struct Process ret = procQ[0]; // Get the first process which will be returned later

    for (int i=0 ; i<numProc-1; i++) {procQ[i] = procQ[i+1];} // Move all processes to the left by 1
    
    struct Process* buffer = calloc(numProc, sizeof(struct Process));
    memcpy(buffer, procQ, sizeof(struct Process)*(numProc-1)); // Copy all processes but the last one to a buffer
    memcpy(procQ, buffer, sizeof(struct Process)*(numProc)); // Copy buffer back to original queue
    for(int i=0 ; i<numProc ; i++){free(procQ[i].cpuBurstTimes);free(procQ[i].ioBurstTimes);} // Free arrays in each struct in buffer
    free(buffer);
    
    return ret;
}

// Function for adding process queue. Takes in queue array, process to be added, and size of queue
void push_back(struct Process* procQ, struct Process proc, int numProc){
    for (int i=0 ; i<numProc; i++){    // Check if the process is in the incorrect state (RUNNING - 0)
        if (procQ[i].state==0) {procQ[i] = proc; break;}   // Add process to queue and terminate rest of for loop
        else {continue;}
    }
}



// =========================================================================================
// ============================= PRINTING + DEBUGGING FUNCTIONS ============================
// =========================================================================================
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


#endif  // PROJECT_PART1_H