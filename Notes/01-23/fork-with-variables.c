/* fork-with-variables.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
  int x = 5;
  int * z = calloc( 20, sizeof( int ) );
  *(z+3) = 12;

  /* When we call fork(), all memory gets effectively duplicated
   *  and copied into the new child process memory image
   *
   * This includes all statically and dynamically allocated
   *  memory, e.g., x, z, p, and the 80 bytes allocated by calloc()
   */

  /* create a new (child) process */
  pid_t p = fork();

  if ( p == -1 )
  {
    perror( "fork() failed" );
    return EXIT_FAILURE;
  }

  /* We see this next line of output twice... */
  printf( "After we call fork()...\n" );

  if ( p == 0 )  /* CHILD PROCESS */
  {
    printf( "CHILD: happy birthday to me! My PID is %d\n", getpid() );
    printf( "CHILD: my parent's PID is %d\n", getppid() );
    x += 120;
    printf( "CHILD: x is now %d\n", x );
#if 0
    free( z );
#endif
  }
  else /* p > 0 --- PARENT PROCESS */
  {
    usleep( 50 );  /* add an artificial delay... */
    printf( "PARENT: my new child process has PID %d\n", p );
    printf( "PARENT: my PID is %d\n", getpid() );
    x -= 5;
    printf( "PARENT: x is now %d\n", x );
  }

  /* TO DO: what happens (with memory) if we move this free() call
   *         up into the if/else block...?
   */
  free( z );

  usleep( 10000 );  /* add another artificial delay... */
  /* ^^^ add this so that the bash shell prompt delays printing */

  return EXIT_SUCCESS;
}
