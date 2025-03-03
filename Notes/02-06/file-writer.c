/* file-writer.c */

/* TO DO: rewrite file-writer.c and file-reader.c to use a pipe */

/* hexdump -C testfile.txt */

/* If we have a command-line argument, which will be the PID value
 *  of the file-reader.out, then we will use signals:
 *
 * SIGUSR1 - file-writer.out will send SIGUSR1 to file-reader.out
 *            to indicate file-reader.out can start reading the file...
 *
 * SIGUSR2 - file_writer.out will send SIGUSR2 to file-reader.out
 *            to indicate file-writer.out is done writing to the file...
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <signal.h>
 
 void signal_handler( int sig )
 {
   printf( "WRITER: Rcvd signal %d\n", sig );
 
   if ( sig == SIGINT )
   {
     printf( "WRITER: Stop hitting CTRL-C\n" );
   }
   else if ( sig == SIGTERM )
   {
     printf( "WRITER: Sorry, I'm not shutting down\n" );
   }
 }
 
 int main( int argc, char ** argv )
 {
   signal( SIGINT, SIG_IGN );    /* ignore SIGINT (CTRL-C) */
   signal( SIGTERM, SIG_IGN );   /* ignore SIGTERM */
 
   /* set SIGINT (CTRL-C) back to default behavior */
   signal( SIGINT, SIG_DFL );
 
   /* redefine SIGINT to call signal_handler() */
   signal( SIGINT, signal_handler );
 
   /* redefine SIGTERM to call signal_handler() */
   signal( SIGTERM, signal_handler );
 
   pid_t reader_pid = 0;
   if ( argc == 2 ) { reader_pid = atoi( *(argv + 1) ); }
 
   char * name = "testfile.txt";
 
   int fd = open( name, O_WRONLY | O_CREAT | O_TRUNC, 0660 );
 
   if ( fd == -1 )
   {
     perror( "WRITER: open() failed" );
     return EXIT_FAILURE;
   }
 
   if ( reader_pid )
   {
     /* send SIGUSR1 to the reader to signal that the shared file exists */
     int rc = kill( reader_pid, SIGUSR1 );
     if ( rc == -1 ) { perror( "WRITER: kill() failed" ); reader_pid = 0; }
   }
 
   for ( int i = 0 ; i < 1024 ; i++ )
   {
     unsigned short value = rand() % 65536;   /* [0,65535] */
     int rc = write( fd, &value, sizeof( unsigned short ) );
     if ( rc != sizeof( unsigned short ) )
     {
       fprintf( stderr, "WRITER: write() failed\n" );
       return EXIT_FAILURE;
     }
     printf( "WRITER: Wrote %u bytes to %s\n", value, name );
     usleep( 5000 );
   }
 
   fsync( fd );   /* hopefully this flushes the cached data */
   close( fd );
 
 #if 1
 sleep( 1 );
 #endif
 
   if ( reader_pid )
   {
     /* send SIGUSR2 to the reader to signal
      * that no more data will be written
      */
     int rc = kill( reader_pid, SIGUSR2 );
     if ( rc == -1 ) { perror( "WRITER: kill() failed" ); reader_pid = 0; }
   }
 
   return EXIT_SUCCESS;
 }
 