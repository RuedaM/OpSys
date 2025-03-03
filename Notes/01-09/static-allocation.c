/* static-allocation.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  int x = 5;  /* x is statically allocated (on the stack) */
              /*   (4 bytes are allocated on the stack)   */

  printf( "x is %d\n", x );
  printf( "sizeof( x ) is %lu bytes\n", sizeof( x ) );
  printf( "sizeof( int ) is %lu bytes\n", sizeof( int ) );


  int * y = NULL; /* y is statically allocated (on the stack) */
                  /*   (8 bytes are allocated on the stack)   */
                  /*   -- memory address is 64-bit or 8 bytes */

  printf( "sizeof( y ) is %lu bytes\n", sizeof( y ) );
  printf( "sizeof( int* ) is %lu bytes\n", sizeof( int* ) );
  printf( "sizeof( char* ) is %lu bytes\n", sizeof( char* ) );
  printf( "sizeof( double* ) is %lu bytes\n", sizeof( double* ) );

#if 0
  printf( "y points to %d\n", *y );
                           /* ^^ seg-fault occurs here! */
#endif

  y = &x;  /* & is the address-of operator */
  printf( "y points to %d\n", *y );

  printf( "x is at memory address %p\n", &x );
  printf( "y is at memory address %p\n", &y );
  printf( "y points to memory address %p\n", y );

  int z = 100;
  printf( "z is %d\n", z );
  printf( "and z is at memory address %p\n", &z );

  return EXIT_SUCCESS;
}
