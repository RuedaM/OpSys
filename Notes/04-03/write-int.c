/* write-int.c */

/* hexdump -C testfile.txt */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>

int main()
{
  char * name = "testfile.txt";

  int fd = open( name, O_WRONLY | O_CREAT | O_TRUNC, 0660 );
  if ( fd == -1 ) { perror( "open() failed" ); return EXIT_FAILURE; }

  int important = 32768;
/*   important = htonl( important ); */
  int rc = write( fd, &important, sizeof( int ) );
  printf( "Wrote %d bytes to fd %d\n", rc, fd );

  close( fd );

  return EXIT_SUCCESS;
}

