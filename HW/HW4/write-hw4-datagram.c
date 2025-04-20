/* write-hw4-datagram.c */

/* hexdump -C testfile.txt */

/* Once you generate the testfile.txt, you can send it directly as
 *  a datagram to your server using some of the following:
 *
 * bash$ cat testfile.txt | netcat -u linux.cs.rpi.edu 8111
 *
 * bash$ cat testfile.txt | netcat -u localhost 8111
 *
 * bash$ cat testfile.txt | netcat -u 127.0.0.1 8111
 *
 * bash$ cat testfile.txt | netcat -u localhost 8111 > output.txt
 * bash$ hexdump -C output.txt
 *
 */

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

  int important = 3;
  important = htonl( important );
  int rc = write( fd, &important, sizeof( int ) );
  printf( "Wrote %d bytes to fd %d\n", rc, fd );

  rc = write( fd, "STARE", 5 );
  printf( "Wrote %d bytes to fd %d\n", rc, fd );

  close( fd );

  return EXIT_SUCCESS;
}

