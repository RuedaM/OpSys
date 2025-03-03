/* rlimit.c */

/* 
 * To look at all of the resource limit values:
 * bash$ ulimit -a
 * bash$ ulimit -u
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

int main()
{
  struct rlimit rl;
  getrlimit( RLIMIT_NPROC, &rl );
  printf( "RLIMIT_NPROC soft limit: %ld\n", rl.rlim_cur );
  printf( "RLIMIT_NPROC hard limit: %ld\n", rl.rlim_max );

  /* lower the RLIMIT_NPROC soft limit to 20 */
  rl.rlim_cur = 20;
  setrlimit( RLIMIT_NPROC, &rl );

  getrlimit( RLIMIT_NPROC, &rl );
  printf( "RLIMIT_NPROC soft limit: %ld\n", rl.rlim_cur );
  printf( "RLIMIT_NPROC hard limit: %ld\n", rl.rlim_max );

  while ( 1 )
  {
    int p = fork();
    if ( p == -1 ){
      perror( "fork() failed" );
      return EXIT_FAILURE;
    }

    printf( "PID %d: fork() worked\n", getpid() );

    /* this gives us some time to shut things down... */
    sleep( 3 );
  }

  return EXIT_SUCCESS;
}
