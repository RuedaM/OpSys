/* shm.c-string */

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
#include <termios.h>

#define SHM_SHARED_KEY 7890

int main(){
    struct termios ttyraw, ttyrestore;
    if(isatty(STDIN_FILENO)){
        tcgetattr(STDIN_FILENO, &ttyrestore);
        cfmakeraw(&ttyraw);
        tcsetattr(STDIN_FILENO, TCSANOW, &ttyraw);
    }
    
    /* create the shared memory segment with a size of 4 bytes */
    key_t key = SHM_SHARED_KEY;
    int shmid = shmget(key, 32, IPC_CREAT | IPC_EXCL | 0660); // 0660 = rw-rw---- = 110110000
    if(shmid==-1) {perror("shmget() failed"); return EXIT_FAILURE;}
    printf("shmget() returned %d\n", shmid);
    /* NOTE that shmget() will zero out the newly created shared memory segment */

    /* attach to the shared memory segment */
    char* x = shmat(shmid, NULL, 0);
    if(x==(void*)-1) {perror("shmat() failed"); return EXIT_FAILURE;}

    /* create a child process --- child process inherits
    *  the pointer to the shared memory segment
    */
    pid_t p = fork();
    if(p==-1) {perror("fork() failed"); return EXIT_FAILURE;}

    if(p==0){
        printf("CHILD: type some characters (enter '!' to end)\n");
        int c;
        char* ptr = x;
        do{
            c = getchar();
            *ptr = c;
            ptr++; //Buffer overflow! Careful!
        }while(c!='!');
    }

    if(p>0){
        while(1){
            printf("PARENT: shared memory contains \"%s\"\n", x);
            sleep(1);
            if (waitpid(p, NULL, WNOHANG)>0) {break;} //Oopsie, check for error
        }
        printf("\rPARENT: all done\n");
    }

    if(isatty(STDIN_FILENO)) {tcsetattr(STDIN_FILENO, TCSANOW, &ttyraw);}


    /* detach from the shared memory segment */
    int rc = shmdt(x);
    if(rc==-1) {perror("shmdt() failed"); return EXIT_FAILURE;}

#if 0
    sleep( 15 );
#endif

#if 0
    if(p>0){
        /* mark the shared memory segment for deletion... */
        printf( "PARENT: removing shared memory segment...\n" );
        if (shmctl( shmid, IPC_RMID, 0)==-1){
            perror("shmctl() failed");
            return EXIT_FAILURE;
        }
    }
#endif

    return EXIT_SUCCESS;
}



#if 0
******** without the waitpid() call... ********
goldsd3@linux-new:~/s25/csci4210$ ./a.out
shmget() returned 98336
PARENT: shared memory contains 0
PARENT: removing shared memory segment...
CHILD: writing my pid 218480 to shared memory...

goldsd3@linux-new:~/s25/csci4210$ ./a.out
shmget() returned 98337
CHILD: writing my pid 218482 to shared memory...
PARENT: shared memory contains 0
PARENT: removing shared memory segment...

goldsd3@linux-new:~/s25/csci4210$ ./a.out
shmget() returned 98338
CHILD: writing my pid 218484 to shared memory...
PARENT: shared memory contains 218484
PARENT: removing shared memory segment...
#endif