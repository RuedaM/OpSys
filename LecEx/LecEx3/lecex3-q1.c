/* lecex3-q1.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <sys/shm.h>



/*
 * Attach to the shared memory segment created by the parent process
 * Convert all uppercase letters in the shared memory segment to lowercase
 * Replace each digit character with a space character
 * Detach from the shared memory segment
 * Exit the child process.
 * 
 * To convey the shm key + the size of the shm segment to your child process,
 *  the parent process will write these two int values to a pipe:
 * 1st int value = size of shm segment
 * 2nd int value = shared memory key
 * 
 * The parent process will display the contents of shared memory after your child process terminates
 * ==> child process must produce no output to stdout
 */
int lecex3_q1_child(int pipefd){
    int size;
    key_t memKey;

    read(pipefd, &size, sizeof(int));
    read(pipefd, &memKey, sizeof(int));

    // Getting ID of shared mem segment
    int shmid = shmget(memKey, size, 0660);
    if (shmid==-1) {perror("ERROR: shmget() failed"); return EXIT_FAILURE;}

    // Attach to shared memory segment
    char* x = shmat(shmid, NULL, 0);
    if (x==(void*)-1) {perror("ERROR: shmat() failed"); return EXIT_FAILURE;}

    char* ptr = x;
    while(*ptr){

        if (isalpha(*ptr)!=0){ // If alphabetical...
            if (islower(*ptr)==0){ // If lowercase...
                *ptr = tolower(*ptr); // Convert to lowercase
            }
        } else if (isdigit(*ptr)){ // If numerical...
            *ptr = ' '; // Replace with space
        }

        ptr++;
    }

    // Detach from shared memory segment
    int rc = shmdt(x);
    if (rc==-1) {perror("shmdt() failed"); return EXIT_FAILURE;}

    return EXIT_SUCCESS;
}