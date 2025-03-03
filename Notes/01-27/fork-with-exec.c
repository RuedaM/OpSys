/* fork-with-exec.c */

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

#if 0
    execl( "/usr/bin/abc", "ls",    "-a",    NULL );

                       /* argv[0], argv[1], argv[2],          argv[3] */
    execl( "/usr/bin/ls", "ls",    "-a",    "nosuchfile.txt", NULL );
#endif

                       /* argv[0], argv[1], argv[2] */
    execl( "/usr/bin/ls", "ls",    "-a",    NULL );

#if 0
    /* FORK BOMB -- FORK BOMB -- FORK BOMB */
    execl( "./a.out", "./a.out", NULL );    /* DO NOT TRY THIS ONE! */
    /* FORK BOMB -- FORK BOMB -- FORK BOMB */
#endif

    /* we should never get to this code! */
    perror( "execl() failed" );
    return EXIT_FAILURE;
  }
  else /* p > 0 --- PARENT PROCESS */
  {
    usleep( 50 );  /* add an artificial delay... */
    printf( "PARENT: my new child process has PID %d\n", p );
    printf( "PARENT: my PID is %d\n", getpid() );

    /* wait (BLOCK) for my child process to complete/terminate */
    int status;
    pid_t child_pid = waitpid( p, &status, 0 );   /* BLOCKING */

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
