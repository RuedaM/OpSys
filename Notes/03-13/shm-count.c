/* shm-count.c */

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
    int shmid = shmget(key, sizeof(int), IPC_CREAT | IPC_EXCL | 0660); // 0660 = rw-rw---- = 110110000
    if(shmid==-1) {perror("shmget() failed"); return EXIT_FAILURE;}

    /* attach to the shared memory (shm) segment */
    int* x = shmat(shmid, NULL, 0);
    if(x==(void*)-1) {perror("shmat() failed"); return EXIT_FAILURE;}

    pid_t p = fork();
    if(p==-1) {perror("fork() failed"); return EXIT_FAILURE;}

    /* both parent and child processes will run the same code below,
    *  both accessing (and changing) the int value in shared memory
    */
    int i, stop = 10000;
    int expected = stop * (stop+1); //  / 2 * 2

    for (i=1 ; i<=stop ; i++){
        printf("%s: adding %d\n", (p>0 ? "PARENT":"CHILD"), i);
        *x += i;
    }

    printf("\n%s: Sum 1..%d (twice) is %d (missing %d)\n",
            p>0 ? "PARENT":"CHILD", stop, *x, expected-*x);


    /* detatch from the shared memory segment */
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