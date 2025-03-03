/* buffering-setvbuf.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* When printing to the terminal (shell) via stdout (fd 1),
 *  a '\n' character will "flush" the stdout buffer,
 *   i.e., output everything that has been stored in
 *    the stdout buffer so far...
 *
 *  ==> this is called line-based buffering
 *
 * When we instead output fd 1 to a file...
 *
 *  bash$ ./a.out > STDOUT.txt
 *
 *  ...the '\n' character no longer flushes the stdout buffer
 *
 *  ==> this is called block-based buffering
 *       (also known as fully buffered)
 *
 * A third type of buffering is non-buffered (unbuffered),
 *  which is what is used for stderr (fd 2)
 */

/* The "fix" to this debugging technique is to
 *  add '\n' characters to the end of each printf()
 */

int main()
{
  setvbuf( stdout, NULL, _IONBF, 0 );

  printf( "HERE0" );    /* stdout buffer: "HERE0" */

  int * x = NULL;

  printf( "HERE1" );    /* stdout buffer: "HERE0HERE1" */

#if 0
  fprintf( stderr, "--do we see this stderr output?" );

  /* this will flush the buffer (regardless of buffering mode) */
  fflush( stdout );
#endif

  *x = 2345;

  printf( "HERE2" );

  printf( "x points to %d\n", *x );

  printf( "HERE3" );

  return EXIT_SUCCESS;
}