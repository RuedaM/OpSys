/* lecex3-q1.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>



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

}