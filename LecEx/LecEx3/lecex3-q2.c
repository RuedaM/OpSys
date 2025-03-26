/* lecex3-q2.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <sys/shm.h>
#include <pthread.h>



void * copy_file( void* arg );

/*
 * You must create a child thread for each filename given as a command-line argument.
 * Once all threads are created, you then must call pthread_join() on each child thread to obtain the number of bytes copied by each thread.
 * 
 * Parallelize these threads to the extent possible.
 * Next, using a separate loop, since pthread_join() is a blocking call, join the threads in the same order that you create them.
*/
int main(int argc, char** argv){
    int childCount = argc-1; // # of children based on # of passed files
    int totalBytes = 0; // Total bytes copied by child threads

    pthread_t* tid = calloc(childCount, sizeof(pthread_t));

    int rc;
    // Create a child thread for each file passed
    for (int i=0 ; i<childCount ; i++){ // For each passed file...
        
        // Create child thread
        char* fileName = *(argv+i+1);
        printf("MAIN: Creating thread to copy \"%s\"\n", fileName);
        rc = pthread_create(tid+i, NULL, copy_file, fileName);   // Changes???

        if (rc!=0){perror( "ERROR: pthread_create failed");}
    }

    // Wait for all child threads to complete/terminate
    for(int i=0 ; i<childCount ; i++){
        int* x;
        pthread_join(*(tid+i), (void**)&x);
        printf("MAIN: Thread completed copying %d bytes for \"%s\"\n", *x, *(argv+i+1));
        totalBytes += *x;
        free(x);
    }

    free(tid);

    printf( "MAIN: Successfully copied %d bytes via %d child thread", totalBytes, childCount );
    if (childCount==1) {printf("\n");}
    else {printf("s\n");}

    return EXIT_SUCCESS;
}