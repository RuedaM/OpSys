/* udp-server-random.c */

/* Application-layer protocol:
 *
 * Server receives a 4-byte int value -- call this cap
 *
 * Server returns a 4-byte int value -- pseudo-random int [0,cap)
 */

/* UDP SERVER example:
 *
 * -- socket()             create a socket (endpoint) for communication
 * -- bind()               bind to (assign) a specific port number
 *
 * -- getsockname()        get socket "name" -- IP addr, port number, etc.
 *
 * -- recvfrom()/sendto()  receive/send datagrams (similar to read()/write())
 *
 */


/* To test this server, you can use the following
   command-line netcat tool:
                                    port number
                                     v
   bash$ netcat -u linux.cs.rpi.edu 41234
                   ^^^^^
                   replace with your hostname

   Note that netcat will act as a client to this UDP server...

   The hostname (e.g., linux.cs.rpi.edu) can also be
   localhost (127.0.0.1); and the port number must match what
   the server reports.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main()
{
  int sd;  /* socket descriptor -- this is actually in the fd table */

  /* create the socket (endpoint) on the server side */
  sd = socket( AF_INET, SOCK_DGRAM, 0 );
  if ( sd == -1 ) { perror( "socket() failed" ); return EXIT_FAILURE; }

  printf( "Server-side UDP socket created on descriptor %d\n", sd );


  struct sockaddr_in udp_server;
  int length = sizeof( udp_server );

  udp_server.sin_family = AF_INET;  /* IPv4 */

  udp_server.sin_addr.s_addr = htonl( INADDR_ANY );
            /* any remote IP can send us a datagram */

  udp_server.sin_port = htons( 0 );  /* htons( 12345 ); */
            /* for bind(), the 0 here means let the OS assign a port number */

  if ( bind( sd, (struct sockaddr *)&udp_server, length ) == -1 )
  {
    perror( "bind() failed" );
    return EXIT_FAILURE;
  }


  /* call getsockname() to obtain the port number that was just assigned */
  if ( getsockname( sd, (struct sockaddr *)&udp_server, (socklen_t *)&length ) == -1 )
  {
    perror( "getsockname() failed" );
    return EXIT_FAILURE;
  }

  printf( "UDP server bound to port %d\n", ntohs( udp_server.sin_port ) );



  /* the code below implements the APPLICATION PROTOCOL */
  while ( 1 )
  {
    int cap = 0;

    struct sockaddr_in remote_client;
    int addrlen = sizeof( remote_client );

    printf( "Blocked waiting for a UDP datagram...\n" );

    /* read a datagram from the remote client side (BLOCKING) */
    int n = recvfrom( sd, &cap, sizeof( int ), 0,
                      (struct sockaddr *)&remote_client,
                      (socklen_t *)&addrlen );

    if ( n == -1 ) { perror( "recvfrom() failed" ); continue; }

    printf( "Rcvd datagram from %s port %d\n",
            inet_ntoa( remote_client.sin_addr ),
            ntohs( remote_client.sin_port ) );

    printf( "Rcvd %d bytes\n", n );
    cap = ntohl( cap );
    printf( "Rcvd: %d\n", cap );

    int value = rand() % cap;
    printf( "Sending int %d\n", value );
    value = htonl( value );
    sendto( sd, &value, sizeof( int ), 0,
            (struct sockaddr *)&remote_client, addrlen );

    /* TO DO: check the return value from sendto() */
  }

  close ( sd );

  return EXIT_SUCCESS;
}
