/* file-reader.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

int file_exists = 0;
int writer_done = 0;

void signal_handler( int sig )
{
  if ( sig == SIGUSR1 )
  {
    file_exists = 1;
  }
  else if ( sig == SIGUSR2 )
  {
    writer_done = 1;
  }
}

int main()
{
  printf( "READER: my PID is %d\n", getpid() );
  signal( SIGUSR1, signal_handler );
  signal( SIGUSR2, signal_handler );

  /* BLOCK here, waiting for file-writer.out to let us know
   *  that the file exists...
   */
  while ( !file_exists ) { sleep( 1 ); }

  char * name = "testfile.txt";

  int fd = open( name, O_RDONLY );
  if ( fd == -1 ) { perror( "READER: open() failed" ); return EXIT_FAILURE; }

  int rc = -1;
  int count = 0;

  do
  {
    unsigned short value;
    rc = read( fd, &value, sizeof( unsigned short ) );
    if ( rc == -1 ) { perror( "READER: read() failed" ); return EXIT_FAILURE; }
    if ( rc == sizeof( unsigned short ) )
    {
      count++;
      printf( "READER: Read %u from %s\n", value, name );
    }
    else { sleep( 1 ); continue; }
  }
  while ( !writer_done || rc != 0 );

  close( fd );

  printf( "READER: Successfully read %d values from %s\n", count, name );

  return EXIT_SUCCESS;
}
