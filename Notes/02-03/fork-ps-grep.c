/* fork-ps-grep.c */

/* A "pipe" is a unidirectional communication channel -- man 2 pipe */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* use fork(), pipe(), and execl() to execute:
 *
 * bash$ ps -ef | grep a.out
 *
 */

int main()
{
  pid_t p1, p2;  /* process ids (PIDs) for ps and grep child processes */

  int pipefd[2];  /* array to hold the two pipe (file) descriptors:
                   *  pipefd[0] is the "read" end of the pipe
                   *  pipefd[1] is the "write" end of the pipe
                   */

  /* create the pipe */
  int rc = pipe( pipefd );
  if ( rc == -1 ) { perror( "pipe() failed" ); return EXIT_FAILURE; }

  /* fd table (BEFORE fork() for both child processes):
   *
   *  0: stdin
   *  1: stdout
   *  2: stderr                  +--------+
   *  3: pipefd[0] <====READ=====| buffer | think of this buffer as a
   *  4: pipefd[1] =====WRITE===>| buffer |  temporary hidden "file"
   *                             +--------+
   */

  printf( "PARENT: going to create the first child process...\n" );

  p1 = fork();
  if ( p1 == -1 ) { perror( "fork() failed" ); return EXIT_FAILURE; }

  if ( p1 == 0 )   /* first CHILD process */
  {
    printf( "CHILD1: going to execute the ps command...\n" );
    close( pipefd[0] );    /* close the read end of the pipe */
    dup2( pipefd[1], 1 );  /* redirect stdout to the write end of the pipe */
    close( pipefd[1] );    /* close the write end of the pipe */

    /* fd table (AFTER fork() in the ps child process):
     *
     *  0: stdin                   +--------+
     *  1: pipefd[1] =====WRITE===>| buffer | think of this buffer as a
     *  2: stderr                  | buffer |  temporary hidden "file"
     *                             +--------+
     */

    execl( "/usr/bin/ps", "ps", "-ef", NULL );
    perror( "execl() failed" );
    return EXIT_FAILURE;
  }

  printf( "PARENT: going to create the second child process...\n" );

  p2 = fork();
  if ( p2 == -1 ) { perror( "fork() failed" ); return EXIT_FAILURE; }

  if ( p2 == 0 )   /* second CHILD process */
  {
    printf( "CHILD2: going to execute the grep command...\n" );
    close( pipefd[1] );    /* close the write end of the pipe */
    dup2( pipefd[0], 0 );  /* redirect stdin to the read end of the pipe */
    close( pipefd[0] );    /* close the read end of the pipe */

    /* fd table (AFTER fork() in the grep child process):
     *                             +--------+
     *  0: pipefd[0] <====READ=====| buffer | think of this buffer as a
     *  1: stdout                  | buffer |  temporary hidden "file"
     *  2: stderr                  +--------+
     */

    execl( "/usr/bin/grep", "grep", "goldsd", NULL );
    perror( "execl() failed" );
    return EXIT_FAILURE;
  }

  /* fd table (in the parent process AFTER fork() for both child processes):
   *
   *  0: stdin
   *  1: stdout
   *  2: stderr                  +--------+
   *  3: pipefd[0] <====READ=====| buffer | think of this buffer as a
   *  4: pipefd[1] =====WRITE===>| buffer |  temporary hidden "file"
   *                             +--------+
   */

  close( pipefd[0] );    /* close the read end of the pipe */
  close( pipefd[1] );    /* close the write end of the pipe */

  /* fd table:
   *
   *  0: stdin
   *  1: stdout
   *  2: stderr
   */

  int status;
  pid_t child_pid = waitpid( p1, &status, 0 );  /* BLOCKING */

  printf( "PARENT: child process %d (ps) terminated...\n", child_pid );

  if ( WIFSIGNALED( status ) )
  {
    printf( "PARENT: ...abnormally (killed by a signal)\n" );
  }
  else if ( WIFEXITED( status ) )
  {
    int exit_status = WEXITSTATUS( status );
    printf( "PARENT: ...normally with exit status %d\n", exit_status );
  }


  child_pid = waitpid( p2, &status, 0 );  /* BLOCKING */

  printf( "PARENT: child process %d (grep) terminated...\n", child_pid );

  if ( WIFSIGNALED( status ) )
  {
    printf( "PARENT: ...abnormally (killed by a signal)\n" );
  }
  else if ( WIFEXITED( status ) )
  {
    int exit_status = WEXITSTATUS( status );
    printf( "PARENT: ...normally with exit status %d\n", exit_status );
  }

  return EXIT_SUCCESS;
}
