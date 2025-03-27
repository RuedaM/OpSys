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


// =========================================================================================
// ======================================== STRUCTS ========================================
// =========================================================================================
// Struct for holding info for a single process
struct Process{
    char* ID; // Process ID (A0 - Z9)
    int state;   // States: IN-MEMORY (0) / IN-QUEUE (1) / PRE-CPU (2) / IN-CPU (3) / POST-CPU (4) / IN-I/O (5) / TERMINATED (6)
    int binding;   // Binding: CPU-Bound (0) / I/O-Bound (1)
    int preemptState;  // States: never preempted (0) / previously preempted (1)
    
    int arrivalTime;   // Process arrival time
    int cpuBurstCount;   // Number of CPU burst for a single process
    // (Note: ioBurstCount = cpuBurstCount-1)
    int idx;   // Variable for indexing CPU bursts and I/O bursts

    int tauInit;   // Predicted CPU burst time (for SJF and SRT) - remains the same for recalculations
    int tau;   // Predicted CPU burst time (for SJF and SRT) - gets decremented
    
    // Containers for storing CPU and I/O burst times
    int* cpuBurstTimes;
    int* ioBurstTimes;

    int cpuBurstCurr; // Currently-active CPU burst
    int ioBurstCurr; // Currently-active I/O burst

    int cpuWaitTime;    // Total CPU wait time for process
    int cpuTurnAround;  // Total CPU turnaround for process
    int preempts;       // Total preempts for process
    int preempted;
    int preempting;
    int withinSlice;    // Total bursts completed within a single time slice for process
};

// Struct for representing a node in a linked list
struct ProcessPlus{
    struct ProcessPlus* prev;
    struct ProcessPlus* next;
    struct Process* p;
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
    char** ret = (char**) malloc(n*sizeof(char*)); //DYNAMIC.MEMORY
    // Remember to free in some way: free(ret)
    if (ret==NULL) {fprintf(stderr, "ERROR: gen_IDs calloc() failed\n"); return NULL;}
    
    int i = 0;
    for (char let='A' ; let<='Z' ; let++) {   // Loop A-Z
        for (char num='0' ; num<='9' ; num++) {   // Loop 0-9
            ret[i] = (char*) malloc(3*sizeof(char)); //DYNAMIC.MEMORY
            // Remember to free in some way: free(ret[i])
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
        int state = 0; //state=IN-MEMORY
        int binding; if(i<n_cpu) {binding=0;} else {binding=1;}
        int preemptState = 0; //preempt state=never preempted
        float arrivalTime = next_exp(lambda, bound, "ceil", 0); // ON THE HANDOUT IT SHOULD SAY TO USE CEIL NOT FLOOR WTF
        int cpuBurstCount = ceil(next_exp(lambda, bound, "-", 1)*32);
        int idx = 0;
        int tauInit = ceil(1/lambda);
        int tau = tauInit;
        
        int* cpuBurstTimes = calloc(cpuBurstCount, sizeof(int)); //DYNAMIC.MEMORY
        int* ioBurstTimes = calloc(cpuBurstCount-1, sizeof(int)); //DYNAMIC.MEMORY
        // Remember to free in some way: free(cpuBurstTimes), free(ioBurstTimes)
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

        int cpuWaitTime = 0;   // Record all wait times for CPU processes
        int cpuTurnAround = 0;  // Record all CPU turnaround time
        int preempts = 0;    // Record number of preempts that occured for the process
        int preempted = 0;
        int preempting = 0;
        int withinSlice = 0;    // Record number of CPU bursts completed with a single time slice

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
        
        struct Process proc = {IDs[i], state, binding, preemptState, arrivalTime, cpuBurstCount, idx, tauInit, tau, cpuBurstTimes,
            ioBurstTimes, cpuBurstCurr, ioBurstCurr, cpuWaitTime, cpuTurnAround, preempts, preempted, preempting, withinSlice};
        allProcesses[i] = proc;
    }

    return allProcesses;
}



// =========================================================================================
// ============================== I/O "QUEUE" HELPER FUNCTIONS =============================
// =========================================================================================
// Function for adding process to queue, sorted by shortest current I/O burst, returns head of queue
struct Process* queue_push_to_end(struct Queue* q, struct Process* p_in){
    //struct ProcessPlus p2 = {NULL, NULL, p_in};
    struct ProcessPlus* node = malloc(sizeof(struct ProcessPlus));
    // Remember to free in some way: free(node)
    if (node==NULL) {fprintf(stderr, "ERROR: queue_push() failed\n"); exit(EXIT_FAILURE);}

    node->p = p_in;
    node->prev = NULL;
    node->next = NULL;

    if (q->head==NULL){ // Empty queue; new node = head
        q->head = node;
        return q->head->p;
    }

    struct ProcessPlus* curr = q->head;
    
    if (p_in->ioBurstCurr < curr->p->ioBurstCurr) { // Filled queue, node has smallest I/O burst; node = head
        node->next = q->head;
        q->head->prev = node;
        q->head = node;
        return q->head->p;
    }

    // Case 3: Traverse the queue to find the correct insertion point

    while (curr->next != NULL && curr->next->p->ioBurstCurr<p_in->ioBurstCurr) { // Filled queue, node has normal I/O burst; node is inserted
        curr = curr->next;
    }
    node->next = curr->next;
    node->prev = curr;
    curr->next = node;

    if (node->next!=NULL) {node->next->prev = node;} // If end isn't reached, change next node's prev ptr

    return q->head->p; // Return ptr to head PROCESS (NOT head node)
}

// Function for removing a process from a queue, which is sorted by shortest current I/O burst
struct Process* queue_pop(struct Queue* q){
    struct ProcessPlus* curr = q->head;

    if (curr->next!=NULL) {curr->next->prev = NULL;}
    q->head = curr->next;

    struct Process* ret = curr->p;
    free(curr);

    return ret;
}

// Function for seeing the status of the whole queue
void io_status(struct Queue* io){
// #if DEBUG_MODE
// printf("...checking I/O queue...\n");
// #endif
    struct ProcessPlus* curr = io->head;
    printf("/---CURRENT I/O QUEUE--------------\n");
    if (io==NULL || io->head==NULL){printf("| -EMPTY-\n");}
    else{
        while (curr!=NULL){
            printf("| Process %s with %dms left\n", curr->p->ID, curr->p->ioBurstCurr);
            curr = curr->next;
        }
    }
    printf("\\----------------------------------\n");
}



// =========================================================================================
// ================================= FCFS HELPER FUNCTIONS =================================
// =========================================================================================
// Function for getting+removing 1st process in queue. Takes in queue array + its size
struct Process* pop(struct Process** procQ, int numProc){
    if (numProc==0 || procQ==NULL) {return NULL;} // Safety check

    struct Process* ret = procQ[0]; // Get the first process which will be returned later
    
    for (int i=0 ; i<numProc-1; i++) {procQ[i] = procQ[i+1];} // Move all processes to the left by 1

    procQ[numProc-1] = NULL; // Avoid dangling pointer
    
    return ret;
}

// // Function for adding process queue. Takes in queue array, process to be added, and size of queue
// void push_back(struct Process* procQ, struct Process proc, int numProc){
//     for (int i=0 ; i<numProc; i++){    // Check if the process is in the incorrect state (RUNNING - 0)
//         if (procQ[i].state==0) {procQ[i] = proc; break;}   // Add process to queue and terminate rest of for loop
//         else {continue;}
//     }
// }




// =========================================================================================
// ================================= RR HELPER FUNCTIONS ===================================
// =========================================================================================
// Function for ...
struct Process* cpuPop(struct Process* procQ){
    if (procQ==NULL) {return NULL;} // Safety check

    struct Process* ret = procQ; // Get CPU process which will be returned later

    procQ = NULL; // Avoid dangling pointer
    
    return ret;
}

// =========================================================================================
// ================================== SJF HELPER FUNCTIONS =================================
// =========================================================================================
// Function for adding Process pointer to priority queue following SJF algorithm (sorting by CPU burst); returns head of priority queue
struct Process* priority_queue_push_SJF(struct Process*** priorityQueue, int priorityQueueLen, struct Process* p_in){
    if (priorityQueue==NULL || p_in==NULL) {fprintf(stderr, "ERROR: p_q_push_SJF() param(s) invalid\n"); return NULL;}

    // Reallocate memory for the priority queue
    struct Process** temp = realloc(*priorityQueue, (priorityQueueLen+1)*sizeof(struct Process*));
    if (temp==NULL) {fprintf(stderr, "ERROR: realloc failed\n"); return NULL;}

    *priorityQueue = temp; // Update the caller's pointer

    int i;
    // Traverse backward to find insertion point
    for (i=priorityQueueLen-1 ; 
         i>=0 && 
         ( (*priorityQueue)[i]->tau>p_in->tau || 
           ( (*priorityQueue)[i]->tau==p_in->tau && 
             strcmp((*priorityQueue)[i]->ID, p_in->ID)>0 ) ) ; 
         i--) {
        (*priorityQueue)[i + 1] = (*priorityQueue)[i]; // Shift right
    }
    (*priorityQueue)[i + 1] = p_in; // Insert new process

    return (*priorityQueue)[0]; // Return queue head
}

// Function for adding Process pointer to priority queue following SJF algorithm (sorting by CPU burst); returns head of priority queue
struct Process* priority_queue_push_front_SJF(struct Process*** priorityQueue, int priorityQueueLen, struct Process* p_in) {
    // Validate input parameters
    if (priorityQueue==NULL || p_in==NULL) {fprintf(stderr, "ERROR: p_q_push_front_SJF() param(s) invalid\n"); return NULL;}

    // Reallocate memory for the priority queue
    struct Process** temp = realloc(*priorityQueue, (priorityQueueLen+1)*sizeof(struct Process*));
    if (temp==NULL) {fprintf(stderr, "ERROR: realloc failed\n"); return NULL;}

    *priorityQueue = temp; // Update the caller's pointer

    for (int i=priorityQueueLen-1 ; i>=0 ; i--) {(*priorityQueue)[i+1] = (*priorityQueue)[i];} // Shift existing elements to the right

    (*priorityQueue)[0] = p_in; // Insert new process at the front

    return (*priorityQueue)[0]; // Return queue head
}

// =========================================================================================
// ================================== SRT HELPER FUNCTIONS =================================
// =========================================================================================
// Function that removes the preempting (from I/O to queue, but immediately switch to CPU after that) process
#include <string.h> // For strcmp()

struct Process* priority_queue_remove_single(struct Process*** priorityQueue, int priorityQueueLen, char* p_in_ID) {
    // Validate input parameters
    if (priorityQueue==NULL || *priorityQueue==NULL || p_in_ID==NULL) {fprintf(stderr, "ERROR: p_q_rmv_single() param(s) invalid\n"); return NULL;}

    // Search for the process with matching ID
    for (int i=0; i<priorityQueueLen ; i++) {
        if (strcmp((*priorityQueue)[i]->ID, p_in_ID)==0) {
            struct Process* removed = (*priorityQueue)[i];

            // Shift elements left to fill the gap
            for (int j=i ; j<priorityQueueLen-1 ; j++) {(*priorityQueue)[j] = (*priorityQueue)[j+1];}

            // Resize the array
            struct Process** temp = realloc(*priorityQueue, (priorityQueueLen-1)*sizeof(struct Process*));
            
            // Update queue pointer (even if realloc fails for size 0)
            if (priorityQueueLen-1>0 && temp!=NULL) {*priorityQueue = temp;}
            else if (priorityQueueLen-1==0) {*priorityQueue = NULL;} // Empty queue

            return removed;
        }
    }

    return NULL; // Process not found
}


// =========================================================================================
// ============================= PRINTING + DEBUGGING FUNCTIONS ============================
// =========================================================================================
// Print relevant process statistics - mostly for debugging
void print_proc(struct Process p){
    printf("||  Process %s is ", p.ID);
    if (p.state==0) {printf("IN-MEMORY");}
    if (p.state==1) {printf("IN-QUEUE");}
    if (p.state==2) {printf("PRE-CPU");}
    if (p.state==3) {printf("IN-CPU");}
    if (p.state==4) {printf("POST-CPU");}
    if (p.state==5) {printf("IN-I/O");}
    if (p.state==6) {printf("TERMINATED");}
    printf(" bound by ");
    if (p.binding==0) {printf("CPU\n");}
    if (p.binding==1) {printf("I/O\n");}
    printf("||  Total CPU Bursts: %d  ", p.cpuBurstCount);
    printf("//  Total I/O Bursts: %d\n", p.cpuBurstCount-1);
    printf("||  ArrTime: %d  ", p.arrivalTime);
    printf("//  Index: %d  ", p.idx);
    printf("//  Tau: %d ", p.tau);
    printf(" (Init. Tau: %d)\n", p.tauInit);
    printf("||  Current CPU Burst: %d  ", p.cpuBurstCurr);
    printf("//  Current I/O Burst: %d\n", p.ioBurstCurr);
    printf("||  Preempts: %d\n", p.preempts);
    // printf("  CPU Burst Times: |");
    // for (int i=0 ; i<p.cpuBurstCount ; i++){printf("%d] %d |", i, p.cpuBurstTimes[i]);} printf("\n");
    // printf("  I/O Burst Times: |");
    // for (int i=0 ; i<p.cpuBurstCount-1 ; i++){printf("%d] %d |", i, p.ioBurstTimes[i]);} printf("\n");

}

void print_all_proc(struct Process* allProcesses, int n){
    printf("//====ALL PROCESSES====================================\n");
    for(int i=0 ; i<n ; i++){
        print_proc(allProcesses[i]);
        printf("\\\\=====================================================\n");
    }
}

void priority_queue_status(struct Process** priorityQueue, int queueLen){
// #if DEBUG_MODE
// printf("\n...checking priority queue...\n");
// #endif
    printf(" [Q");
    if (queueLen==0) {printf(" empty");}
    else{
        for(int i=0 ; i<queueLen ; i++) {
            if (priorityQueue[i]==NULL) {printf(" ERROR_NULL");}
            else if (priorityQueue[i]->ID==NULL) {printf(" ERROR_ID_NULL");}
            else {
                if (priorityQueue[i]->state==1) {printf(" %s", priorityQueue[i]->ID);}
            }
        }
    }
    printf("]\n");
}


#endif  // PROJECT_PART1_Hf