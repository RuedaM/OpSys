/* fork-pass-by-fork.c */

/* When we call fork(), the state of the process/variables is
 *  effectively passed (copied) to the child process...
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void block_on_waitpid( pid_t p );
void child_process( char * data, int i );



int main(){
  char * data = calloc( 27, sizeof( char ) );

  /*                       1111111111222222 */
  /*      index: 01234567890123456789012345 */
  strcpy( data, "abcdefghijklmnopqrstuvwxyz" );

  /* this index variable i will indicate to the child process
   *  which chunk of data to convert to uppercase and display
   *   (each child process will work on a substring of length 7)
   */
  int i = 5;

  pid_t p1, p2;

  /* create the first child process */
  p1 = fork();  /* i == 5 */
  if (p1==-1) {
    perror("fork() failed");
    return EXIT_FAILURE;
  }

  if (p1==0){  /* first CHILD process executes here... */
    printf( "CHILD %d: happy birthday to me!\n", getpid() );
    child_process( data, i );  /* <==== pass-by-fork occurs here */
  }

  else if (p1>0){  /* PARENT process continues to execute here... */
    usleep(100);
    printf("PARENT: my first child PID is %d\n", p1);

    i = 16;

    /* create the second child process */
    p2 = fork();  /* i == 16 */
    if (p2==-1) {perror( "fork() failed"); return EXIT_FAILURE;}

    if (p2==0){  /* second CHILD process executes here... */
      printf("CHILD %d: happy birthday to me!\n", getpid());
      child_process(data, i);  /* <==== pass-by-fork occurs here */
    
    }else if (p2>0){  /* PARENT process continues to execute here... */
      usleep(100);
      printf("PARENT: my second child PID is %d\n", p2);
      block_on_waitpid(p1);
      block_on_waitpid(p2);

      printf("PARENT: data: \"%s\"\n", data);
    }
  }

  free(data);

  return EXIT_SUCCESS;
}

void block_on_waitpid(pid_t p)
{
  /* wait (BLOCK) for my child process to complete/terminate */
  int status;
  pid_t child_pid = waitpid(p, &status, 0);  /* BLOCKING */

  printf("PARENT: child process %d terminated...\n", child_pid);

  if (WIFSIGNALED(status)){
    printf( "PARENT: ...abnormally (killed by a signal)\n" );
  
  }else if (WIFEXITED(status)){
    int exit_status = WEXITSTATUS( status );
    printf( "PARENT: ...normally with exit status %d\n", exit_status );
  }
}



void child_process(char* data, int i){
  for ( int j=0 ; j<7 ; j++){
    if (isalpha(*(data+i+j))){   /* see also islower() */
      *(data+i+j) = toupper( *(data+i+j) );
    }
  }

  printf( "CHILD %d: data: \"%s\"\n", getpid(), data );
}