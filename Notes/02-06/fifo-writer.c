/* fifo-writer.c */

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

  /* create the fifo in the writer... */
  int rc = mkfifo( fifo, 0660 );   /* see fd-write.c */
  if ( rc == -1 ) { perror( "WRITER: mkfifo() failed" ); return EXIT_FAILURE; }

  printf( "WRITER: Created fifo at %s\n", fifo );


  int fd = open( fifo, O_WRONLY );
  if ( fd == -1 ) { perror( "WRITER: open() failed" ); return EXIT_FAILURE; }

  /* fd table:
   *
   *  0: stdin
   *  1: stdout                  IN MEMORY:
   *  2: stderr                  +--------+
   *  3: fd =========WRITE======>| buffer |   /tmp/fifo1234
   *                             | buffer |
   *                             +--------+
   */

  printf( "WRITER: Opened fifo %s on fd %d for writing\n", fifo, fd );

  /* write data to the file */
  int bytes_written = write( fd, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26 );
  if ( bytes_written == -1 )
  {
    perror( "WRITER: write() failed on the fifo" );
    return EXIT_FAILURE;
  }

  printf( "WRITER: Wrote %d bytes\n", bytes_written );

#if 1
  sleep( 10 );
#endif

  /* TO DO: write more data... */

  close( fd );

#if 1
  printf( "WRITER: Marking this FIFO for deletion...\n" );

  /* mark the fifo for deletion now that we are done with it... */
  rc = unlink( fifo );
  if ( rc == -1 ) { perror( "WRITER: unlink() failed" ); return EXIT_FAILURE; }
#endif

  return EXIT_SUCCESS;
}
