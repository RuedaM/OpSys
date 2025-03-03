/* dynamic-allocation.c */

/* compile using: gcc -Wall -Werror -g dynamic-allocation.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
  /* dynamically allocate 16 bytes on the runtime heap */
  char * path = malloc( 16 );

  if ( path == NULL )
  {
    perror( "malloc() failed" );
    return EXIT_FAILURE;
  }

  /* dynamically allocate 16 bytes on the runtime heap */
  char * path2 = malloc( 16 );

  if ( path2 == NULL )
  {
    perror( "malloc() failed" );
    return EXIT_FAILURE;
  }

  printf( "sizeof path is %lu\n", sizeof( path ) );
  printf( "sizeof path2 is %lu\n", sizeof( path2 ) );

  strcpy( path, "/cs/goldsd/s25/" );
  printf( "path is \"%s\" (strlen is %lu)\n", path, strlen( path ) );

  strcpy( path, "/cs/goldsd/s25/" );
  printf( "path is \"%s\" (strlen is %lu)\n", path, strlen( path ) );
  strcpy( path2, "ABCDEFGHIJKLMNOP" );  /* 1-byte overflow */
  printf( "path2 is \"%s\" (strlen is %lu)\n", path2, strlen( path2 ) );

  /* TO DO: try creating larger buffer overflows and observe the behavior */
  /*         in the output, also what valgrind tells you...               */

#if 0
  strcpy( path2, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" );
  printf( "path2 is \"%s\" (strlen is %lu)\n", path2, strlen( path2 ) );
#endif

  free( path );
  free( path2 );

  return EXIT_SUCCESS;
}
