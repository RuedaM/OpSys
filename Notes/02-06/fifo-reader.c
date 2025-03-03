/* fifo-reader.c */

/* man 3 mkfifo; man 7 pipe */

/* A "pipe" is a unidirectional communication channel -- man 2 pipe */
/*               ^^^                                                */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
  char * fifo = "/tmp/fifo1234";

  int fd = open( fifo, O_RDONLY );
  if ( fd == -1 ) { perror( "READER: open() failed" ); return EXIT_FAILURE; }

  /* fd table:
   *
   *  0: stdin
   *  1: stdout                  IN MEMORY:
   *  2: stderr                  +--------+
   *  3: fd <========READ========| buffer |   /tmp/fifo1234
   *                             | buffer |
   *                             +--------+
   */

  printf( "READER: Opened fifo %s on fd %d for reading\n", fifo, fd );

  char * buffer = calloc( 5, sizeof( char ) );

  while ( 1 )
  {
    /* read data from the fifo */
    int bytes_read = read( fd, buffer, 4 );    /* BLOCKING */

    if ( bytes_read == -1 )
    {
      perror( "READER: read() failed on the fifo" );
      return EXIT_FAILURE;
    }

    if ( bytes_read == 0 )
    {
      printf( "READER: read() returned 0; writer process closed its write fd\n" );
      break;
    }

    *(buffer + bytes_read) = '\0';
    printf( "READER: Read %d bytes: \"%s\"\n", bytes_read, buffer );
  }

  free( buffer );

  close( fd );

  return EXIT_SUCCESS;
}
