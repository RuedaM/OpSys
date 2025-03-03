/* static-allocation.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
                           /*        [0] [1] [2] [3] [4] [5]   */
                           /*       +---+---+---+---+---+----+ */
  char name[5] = "David";  /* name: | D | a | v | i | d | \0 | */
                           /*       +---+---+---+---+---+----+ */

  printf( "Hi, %s\n", name );
#if 0
  name[3] = 'i';
  printf( "Hi, %s (corrected?)\n", name );
#endif

  char xyz[5] = "QRSTU";

  printf( "Hi again, %s\n", name );
  printf( "xyz is %s\n", xyz );

  /* TO DO: correct the bugs above by allocating [6] instead of [5] */
  /*        ...try different values, e.g., [3], [300], etc.?        */
  /* TO DO: can you make the above code seg-fault...?               */

#if 0
  /* The string below is immutable because it is in read-only memory */
  char * cptr = "ABCDEFgHIJKLMNOPQRSTUVWXYZ";   /* implied '\0' */
#endif

  /* This "works" because the compiler figures out that we need 27 bytes */
  char cptr[] = "ABCDEFgHIJKLMNOPQRSTUVWXYZ";   /* implied '\0' */

  printf( "cptr points to \"%s\"\n", cptr );
  cptr[6] = 'G';
  printf( "cptr points to \"%s\"\n", cptr );

  return EXIT_SUCCESS;
}
