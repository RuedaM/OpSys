/* fork-with-waitpid-WNOHANG.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
  /* create a new (child) process */
  pid_t p = fork();

  if ( p == -1 )
  {
    perror( "fork() failed" );
    return EXIT_FAILURE;
  }

  if ( p == 0 )  /* CHILD PROCESS */
  {
    printf( "CHILD: happy birthday to me! My PID is %d\n", getpid() );
    printf( "CHILD: my parent's PID is %d\n", getppid() );

#if 1
    /* uncomment this to add delay for us to send a kill signal... */
    /* ...or for us to see waitpid( ..., WNOHANG ) return 0 in the parent */
    sleep( 10 );
#endif

#if 0
    int * q = NULL;
    *q = 1234;        /* man 7 signal */
#endif

    return -5;
    return 0x3f;
    return 12345;  /* how does this end up being 57...?! */
    return EXIT_FAILURE;
    return 12;
  }
  else /* p > 0 --- PARENT PROCESS */
  {
    usleep( 50 );  /* add an artificial delay... */
    printf( "PARENT: my new child process has PID %d\n", p );
    printf( "PARENT: my PID is %d\n", getpid() );

#if 0
    sleep( 20 );   /* add this delay to "see" the zombie process... */
#endif

    /* wait (BLOCK) for my child process to complete/terminate */
    int status;
#if 0
    pid_t child_pid = waitpid( p, &status, 0 );   /* BLOCKING */
#endif
    pid_t child_pid = 0;
    do
    {
      child_pid = waitpid( p, &status, WNOHANG ); /* NON-BLOCKING */

      if ( child_pid == 0 )
      {
        printf( "PARENT: still waiting for child process to end...\n" );
        sleep( 1 );
      }
    }
    while ( child_pid == 0 );

    printf( "PARENT: child process %d terminated...\n", child_pid );

    if ( WIFSIGNALED( status ) )
    {
      printf( "PARENT: ...abnormally (killed by a signal)\n" );
    }
    else if ( WIFEXITED( status ) )
    {
      int exit_status = WEXITSTATUS( status );
      printf( "PARENT: ...normally with exit status %d\n", exit_status );
    }
  }

  usleep( 10000 );  /* add another artificial delay... */
  /* ^^^ add this so that the bash shell prompt delays printing */

  return EXIT_SUCCESS;
}