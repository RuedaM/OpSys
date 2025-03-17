/* shm-shared-mem-x.c */

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
    int shmid = shmget(key, sizeof(int), IPC_CREAT | IPC_EXCL | 0660);
                                                            /* rw-rw---- */
    if(shmid==-1) {perror("shmget() failed"); return EXIT_FAILURE;}

    printf("shmget() returned %d\n", shmid);

    /* attach to the shared memory segment */
    int * x = shmat(shmid, NULL, 0);
    if(x==(void*)-1) {perror("shmat() failed"); return EXIT_FAILURE;}

    *x = 5;

    /* create a child process --- child process inherits
    *  the pointer to the shared memory segment
    */
    pid_t p = fork();
    if(p==-1) {perror("fork() failed"); return EXIT_FAILURE;}

    // Both parent and child processes will run the samecode below,
    //  both accessing (+ changing) the int value in shared memory

    int i, stop = 1000;
    int expected = stop*(stop+1);
    for(i=1 ; i<=stop ; i++){
        printf("%s adding %d\n", (p>0 ? "PARENT" : "CHILD"), i);
        *x += i;
    }

    printf("\n%s: Sum 1..%d (twice) is %d (missing %d)\n",
        p>0 ? "PARENT" : "CHILD", stop, *x, (expected - *x));
    
    /* detach from the shared memory segment */
    int rc = shmdt(x);
    if (rc==-1) {perror( "shmdt() failed"); return EXIT_FAILURE;}

#if 0
    sleep( 15 );
#endif

#if 1
    if (p>0){
        /* mark the shared memory segment for deletion... */
        if (shmctl(shmid, IPC_RMID, 0)==-1){
            perror("shmctl() failed");
            return EXIT_FAILURE;
        }
    }
#endif

    return EXIT_SUCCESS;
}