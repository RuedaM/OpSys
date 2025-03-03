/* simple.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define BUFFER_MAX 32

int main()
{
  /* one char == one byte */

  /*                      [0] [1] [2] [3]  [4]   */
  /*                     +---+---+---+----+----+ */
  /* C string in memory: | H | i | . | \n | \0 | */
  /*                     +---+---+---+----+----+ */
  /*                                        ^^   */
  /*                       '\0' is end-of-string */

  printf( "Hi.\n" );
  printf( "BUFFER_MAX is %d\n", BUFFER_MAX );

  int i;
  for ( i = 1 ; i < BUFFER_MAX / 4 ; i++ ){
    printf( "%3d %f %20.15f\n", i, sqrt( i ), sqrt( i ) );
    printf( "%03d %f %-20.15f\n", i, sqrt( i ), sqrt( i ) );
    printf( "%-3d %f %20.20f\n", i, sqrt( i ), sqrt( i ) );
    printf( "%-3d %f %20.200f\n", i, sqrt( i ), sqrt( i ) );
  }

  float j;   /* float j = 123.45; */
  printf( "printf prints j: %f\n", j );

  return EXIT_SUCCESS;
}
