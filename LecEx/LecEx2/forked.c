/* forked.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>


/* implement these functions in lecex2-q1.c */
int lecex2_child( unsigned int n );
int lecex2_parent();

int main()
{
  unsigned int n = 128;  /* or some other non-negative value ... */
  int rc;

  //printf( "PID %d: BEFORE fork()\n", getpid() );

  /* create a new (child) process */
  pid_t p = fork();

  if ( p == -1 )
  {
    perror( "fork() failed" );
    return EXIT_FAILURE;
  }

  //printf( "PID %d: AFTER fork()\n", getpid() );

  if ( p == 0 )
  {
    rc = lecex2_child( n );
  }
  else /* p > 0 */
  {
    rc = lecex2_parent();
  }

  return rc;
}
