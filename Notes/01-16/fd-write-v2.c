/* fd-read-v2.c */

/**
 *  file descriptor (fd)
 *
 *  -- a small non-negative integer used in a variety of system calls
 *      to refer to an open file (i.e., file stream or byte stream)
 *
 *  -- each process has a file descriptor table associated with it
 *      that keeps track of its open files
 *
 *  fd        C++   Java         C
 *  0 stdin   cin   System.in    scanf(), read(), getchar(), ...
 *  1 stdout  cout  System.out   printf(), write(), putchar(), ...
 *  2 stderr  cerr  System.err   fprintf( stderr, "ERROR: ...\n" );
 *                               perror( "open() failed" );
 *
 *  stdout and stderr (by default) are both displayed on the terminal
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(){
    char* name = "testfile.txt";

    int fd = open(name, O_RDONLY);

    if (fd==-1){
        perror( "open() failed" );
        return EXIT_FAILURE;
    }

    printf("fd is %d\n", fd);

    /* fd table for this running process:
    *
    *  0 stdin
    *  1 stdout
    *  2 stderr
    *  3 testfile.txt (O_RDONLY)
    */

    /* we are assuming that we have character data... */
    char buffer[20];
    int rc = read(fd, buffer, 11);
    buffer[rc] = '\0';  /* null-terminate the data read in... */
    printf("read() returned %d -- read \"%s\"\n", rc, buffer);

    rc = read(fd, buffer, 11);
#if 0
    buffer[rc] = '\0';  /* null-terminate the data read in... */
#endif
    printf("read() returned %d -- read \"%s\"\n", rc, buffer);

    rc = read(fd, buffer, 11);
#if 0
    buffer[rc] = '\0';  /* null-terminate the data read in... */
#endif
    printf("read() returned %d -- read \"%s\"\n", rc, buffer);

    rc = read(fd, buffer, 11);
    buffer[rc] = '\0';  /* null-terminate the data read in... */
    printf("read() returned %d -- read \"%s\"\n", rc, buffer);

#if 0
    sleep( 15 );  /* suspend this process for 15 seconds */
    /* TO DO: try to rename (mv) the file...
    *        try to remove (rm) the file...
    */
#endif

    rc = read(fd, buffer, 11);
    buffer[rc] = '\0';  /* null-terminate the data read in... */
    printf("read() returned %d -- read \"%s\"\n", rc, buffer);

    close(fd);

    rc = read(fd, buffer, 11);
    if (rc == -1){perror( "read() failed" );}
    buffer[rc] = '\0';  /* null-terminate the data read in... */
    /* what does buffer[-1] do here...? */
    printf("read() returned %d -- read \"%s\"\n", rc, buffer);

    return EXIT_SUCCESS;
}
