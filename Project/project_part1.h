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

// Struct for holding info for a single process
struct Process{
    char* ID; // Process ID (A0 - Z9)
    int state;   // States: RUNNING (0) / READY (1) / WAITING (2)
    int binding;   // Binding: CPU-Bound (0) / I/O-Bound (1)
    
    int arrivalTime; // Process arrival time
    int cpuBurstCount;   // Number of CPU burst for a single process
    // (Note: ioBurstCount = cpuBurstCount-1)
    
    // Containers for storing CPU and I/O burst times
    int* cpuBurstTimes;
    int* ioBurstTimes;
};

// Generate IDs for processes
char** gen_IDs(int n){
    char** ret = (char**) malloc(n*sizeof(char*));
    if (ret==NULL) {perror("ERROR: gen_IDs calloc failed"); return NULL;}
    
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

// Function for getting and removing the first process in queue. Takes in the queue array and size of the queue array
struct Process pop(struct Process* procQ, int numProc){
    struct Process ret = procQ[0];  // Get the first process which will be returned later

    for (int i=0 ; i<numProc-1; i++ ){procQ[i] = procQ[i+1];} // Move all processes to the left by 1
    
    struct Process* buffer = calloc(numProc, sizeof(struct Process));
    memcpy(buffer, procQ, sizeof(struct Process)*(numProc-1));  // Copy all processes but the last one to a buffer
    memcpy(procQ, buffer, sizeof(struct Process)*(numProc));    // Copy buffer back to original queue
    free(buffer);
    
    return ret;
}

// Function for pushing a process into queue. Takes in the queue array, process to be added, and size of the queue array
void push_back(struct Process* procQ, struct Process proc, int numProc){
    for (int i=0 ; i<numProc; i++ ){    // Check if the process is in the incorrect state (RUNNING - 0)
        if (procQ[i].state == 0){procQ[i] = proc; break;}   // Add process to queue and terminate rest of for loop
        else {continue;}
    }
}