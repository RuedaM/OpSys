/* shm-show-addr.c */

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
    /* create the shared memory segment with a size of 4 bytes */
    key_t key = SHM_SHARED_KEY;
    int shmid = shmget(key, sizeof(int), IPC_CREAT | /* IPC_EXCL | */ 0660); // 0660 = rw-rw---- = 110110000
    if (shmid==-1) {perror("shmget() failed"); return EXIT_FAILURE;}
    printf("shmget() returned %d\n", shmid);
    /* NOTE that shmget() will zero out the newly created shared memory segment */

    /* attach to the shared memory segment */
    int* x = shmat(shmid, NULL, 0);
    if (x==(void *)-1) {perror("shmat() failed"); return EXIT_FAILURE;}

    /* create a child process --- child process inherits
    *  the pointer to the shared memory segment
    */
    pid_t p = fork();
    if (p==-1) {perror("fork() failed"); return EXIT_FAILURE;}

    if (p==0){
        printf("CHILD: shared memory is at %p\n", x);
        printf("CHILD: writing my pid %d to shared memory...\n", getpid());
        *x = getpid();
    }

    if (p>0){
        printf("PARENT: shared memory is at %p\n", x);
#if 0
        /* this is the only synchronization mechanism that's
        *  in this code --- synchronizes write and read on
        *   the shared memory segment
        */
        waitpid( p, NULL, 0 );
#endif
        printf("PARENT: shared memory contains %d\n", *x);
    }


    /* detach from the shared memory segment */
    int rc = shmdt(x);
    if (rc == -1) {perror( "shmdt() failed"); return EXIT_FAILURE;}

#if 0
    sleep(15);
#endif

#if 0
    if (p>0){
        /* mark the shared memory segment for deletion... */
        printf("PARENT: removing shared memory segment...\n");
        if (shmctl(shmid, IPC_RMID, 0)==-1) {perror( "shmctl() failed" ); return EXIT_FAILURE;}
    }
#endif

    return EXIT_SUCCESS;
}