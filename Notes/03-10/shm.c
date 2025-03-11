/* shm.c */

// Create ahred memory egment, then

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/shm.h>

#define SHM_SHARED_KEY 7890

int main(){
    // Create shared memory segment with size of 4 bytes
    key_t key = SHM_SHARED_KEY;
    int shmid = shmget(key, sizeof(int), IPC_CREAT | IPC_EXCL | 0660); // Permissions: rw-rw----
    if (shmid==-1) {perror("shmget() failed"); return EXIT_FAILURE;}
    printf("shmget() returned %d\n", shmid);

    // Attach to shared memory segment
    int* x = shmat(shmid, NULL, 0);
    if(x==(void*)-1) {perror("shmat() failed"); return EXIT_FAILURE;}

    // Create child process --shared child process inherits
    //  the pointer to shared memory segment
    pid_t p = fork();
    if(p==-1) {perror("fork() failed"); return EXIT_FAILURE;}
    if(p==0){
        printf("CHILD: writing my pid %d to shared memory...\n", getpid());
        *x = getpid();
    }
    if(p>0){
        waitpid(p, NULL, 0);
        printf("PARENT: shared memory contains %d\n", *x);

    }
    


    // Detatch from shared memory segment
    int rc = shmdt(x);
    if(rc==-1) {perror("shmdt() failed"); return EXIT_FAILURE;}

    // Mark shared memory segment for deletion
    printf("removing shared memory segment...\n");
    if(shmctl(shmid, IPC_RMID, 0)==-1) {perror("shmctl() failed"); return EXIT_FAILURE;}

    return EXIT_SUCCESS;
}