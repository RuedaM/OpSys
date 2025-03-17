/* shm-sync.c */

/* TO DO: write a separate program to attach to the shared memory segment
 *         created below
 *
 *        can you scale this up to have multiple child processes?
 *
 *        can you scale this up to have multiple child reader processes?
 */

/* create a shared memory segment, then create a child process
 *  such that both parent and child are attached to the shared memory
 */

/* you can use the shell to view/remove shared memory segments:
 *
 * bash$ ipcs
 * bash$ ipcs -m
 *
 * and this will remove all shared memory segments:
 *
 * bash$ ipcrm -a
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SHM_SHARED_KEY 7890

int main(){
    /* create the shared memory segment with a size of 8 BYTES NOW */
    key_t key = SHM_SHARED_KEY;
    int shmid = shmget(key, 2*sizeof(int), IPC_CREAT | /* IPC_EXCL | */ 0660);
                                                                    /* rw-rw---- */
    if (shmid==-1) {perror("shmget() failed"); return EXIT_FAILURE;}

    printf("shmget() returned %d\n", shmid);

    /* NOTE that shmget() will zero out the newly created shared memory segment */


    /* attach to the shared memory segment */
    int* x = shmat( shmid, NULL, 0 );
    if (x==(void *)-1) {perror("shmat() failed"); return EXIT_FAILURE;}


    /* create a child process --- child process inherits
    *  the pointer to the shared memory segment
    */
    pid_t p = fork();
    if (p==-1) {perror("fork() failed"); return EXIT_FAILURE;}

    if (p==0){
        printf("CHILD: writing my pid %d to shared memory...\n", getpid());
        *x = getpid();

        /* notify the parent/reader process that x is valid */
        *(x+1) = 1;

        /* busy wait loop... */
        while (*(x+1)==1) {/* nop */}

        printf( "CHILD: writing 2345 to shared memory...\n" );
        *x = 2345;

        /* notify the parent/reader process that x is valid */
        *(x+1) = 1;
    }

    if (p>0){
        /* busy wait loop... */
        while(*(x+1)==0) {/* nop */}

        printf("PARENT: shared memory contains %d\n", *x);

        /* notify the child/writer process that it is okay to write new data */
        *(x+1) = 0;

        /* busy wait loop... */
        while(*(x+1)==0) {/* nop */}

        printf("PARENT: shared memory contains %d\n", *x);
    }


    /* detach from the shared memory segment */
    int rc = shmdt(x);
    if (rc==-1) { perror("shmdt() failed" ); return EXIT_FAILURE;}

#if 0
    sleep(15);
#endif

#if 1
    if (p>0){
        /* mark the shared memory segment for deletion... */
        printf("PARENT: removing shared memory segment...\n");
        if (shmctl(shmid, IPC_RMID, 0)==-1) {perror( "shmctl() failed" ); return EXIT_FAILURE;}
    }
    #endif

    return EXIT_SUCCESS;
}