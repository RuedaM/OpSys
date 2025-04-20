/* endian.c */

/* to look at the individual bytes of the output file:

   bash$ hexdump -C endian.dat
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <arpa/inet.h>
 
 int main()
 {
   short x = 720;   /* 2-byte representation */
     /* binary: 0000 0010 1101 0000  (i.e., 512+128+64+16) */
     /*    hex:    0    2    d    0                        */
 
     /* little endian:  d0 02 */
     /*    big endian:  02 d0 */
 
   int fd = open( "endian.dat", O_WRONLY | O_CREAT | O_TRUNC, 0660 );
   if ( fd == -1 ) { perror( "open() failed" ); return EXIT_FAILURE; }
 
   int rc = write( fd, "x:", 2 );
   if ( rc == -1 ) { perror( "write() failed" ); return EXIT_FAILURE; }
 
   rc = write( fd, &x, sizeof( short ) );
   if ( rc == -1 ) { perror( "write() failed" ); return EXIT_FAILURE; }
 
   /* convert x from host to network byte order */
   short z = htons( x );  /* also see ntohs() */
 
   rc = write( fd, "--z:", 4 );
   if ( rc == -1 ) { perror( "write() failed" ); return EXIT_FAILURE; }
 
   rc = write( fd, &z, sizeof( short ) );
   if ( rc == -1 ) { perror( "write() failed" ); return EXIT_FAILURE; }
 
   close( fd );
 
   return EXIT_SUCCESS;
 }