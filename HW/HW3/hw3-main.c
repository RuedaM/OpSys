/* hw3-main.c */

/* bash$ gcc -Wall -Werror -o hw3.out hw3-main.c hw3.c */

/*
 * gcc -Wall -Werror -D -o hw3.out hw3-main.c hw3.c
 * gcc -Wall -Werror -D NO_PARALLEL -o hw3.out hw3-main.c hw3.c
 * gcc -Wall -Werror -D QUIET -o hw3.out hw3-main.c hw3.c
 * gcc -Wall -Werror -D DEBUG_MODE -o hw3.out hw3-main.c hw3.c
 * gcc -Wall -Werror -D NO_PARALLEL -D DEBUG_MODE -o hw3.out hw3-main.c hw3.c
 * gcc -Wall -Werror -D QUIET -D DEBUG_MODE -o hw3.out hw3-main.c hw3.c
 * gcc -Wall -Werror -D NO_PARALLEL -D QUIET -o hw3.out hw3-main.c hw3.c
 * gcc -Wall -Werror -D NO_PARALLEL -D QUIET -D DEBUG_MODE -o hw3.out hw3-main.c hw3.c
 * 
 * valgrind -s --leak-check=full ./hw3.out <ARGS>
 * 3 3 0 0 -- no parallel
 * 3 4 1 0 -- no parallel
 * 3 4 1 1 -- no parallel
 * 3 4 1 0 -- parallel
 * 3 8 0 0 -- quiet, no parallel
 * 3 5 1 2 -- paral
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

long next_thread_number;
int max_squares;
int total_open_tours;
int total_closed_tours;

/* write the solve() function and place all of your code in hw3.c */
int solve( int argc, char ** argv );

int main( int argc, char ** argv )
{
  next_thread_number = 1;  /* or other positive long value */
  max_squares = total_open_tours = total_closed_tours = 0;

        
  int rc = solve( argc, argv );

  /* on Submitty, there will be more code here that validates
   *  the global variables when your solve() function returns...
   */

  return rc;
}