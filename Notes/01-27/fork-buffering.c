/* fork-buffering.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
  /* The stdout buffer is copied to the child process
   *  when we call fork()...
   */
  printf( "GOOD MORNING." );   /* with or without '\n' ... */

#if 0
  fflush( stdout );  /* always be fd 1 */
  --or--
  fflush( NULL );
#endif

  /* If we redirect to a file (./a.out > output.txt), then
   *  this stdout buffer contains "GOOD MORNING." and so on
   *   and this buffer gets copied to the child process.
   *
   * Since we are in a block-based buffering mode, there are
   *  now only two possible outputs for output.txt...
   */

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
  }
  else /* p > 0 --- PARENT PROCESS */
  {
    usleep( 50 );  /* add an artificial delay... */
    printf( "PARENT: my new child process has PID %d\n", p );
    printf( "PARENT: my PID is %d\n", getpid() );
  }

#if 0
  sleep( 20 );
#endif

  usleep( 10000 );  /* add another artificial delay... */
  /* ^^^ add this so that the bash shell prompt delays printing */

  return EXIT_SUCCESS;
}

/*
 * goldsd@linux:~/s25/csci4210$ ./a.out 
 * CHILD: happy birthday to me! My PID is 3109077
 * PARENT: my new child process has PID 3109077
 * PARENT: my PID is 3109076
 * CHILD: my parent's PID is 3109076
 * goldsd@linux:~/s25/csci4210$ ./a.out 
 * PARENT: my new child process has PID 3109079
 * PARENT: my PID is 3109078
 * CHILD: happy birthday to me! My PID is 3109079
 * CHILD: my parent's PID is 3109078
 * goldsd@linux:~/s25/csci4210$ 
 *
 * What are all the possible outputs for this code?
 *
 *                     GOOD MORNING.
 *                      p = fork()
 *                       /    \
 *            <PARENT>  /      \  <CHILD>
 *                     /        \
 * PARENT: my new child...     CHILD: happy birthday to me! ...
 * PARENT: my PID is...        CHILD: my parent's PID is...
 * 
 * (1) lines shown above in the <PARENT> section occur in that
 *      given order; same for the <CHILD> section
 *
 * (2) lines shown above in the <PARENT> section could interleave
 *      with the lines shown above in the <CHILD> section
 */





