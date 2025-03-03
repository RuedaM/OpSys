/* fd-write.c */

/* hexdump -C testfile.txt */

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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



typedef struct{
  unsigned int rin;   /* e.g., 660000001 */
  char rcsid[20];     /* e.g., "goldsd3" */
  /* ... */
  char * advisor;
}
student_t;



int main(){
    char* name = 'testfile.txt';

    /* 
     * Attempt to open this file:
     * O_WRONLY for writing
     * O_CREAT crateas the file, if necessary
     * O_TRUNC truncates the fie if it already existss
     * (also see O_APPEND)
     */
    int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0660);
    /* 
     * Note that last argumment:
     * leading 0 ==> octal
     * 0660 ==> 110 110 000
     * 
     * 0660 ==> 110 110 000
     *          rwx rwx rwx
     *          ^^^ ^^^ ^^^
     *          |   |   |
     *          |   |  no permissions for other/world
     *          |   |
     *          |  rw for group permissions
     *          |
     *          rw for file owner
     */
    
    if (fd==-1){
        perror("open() failed");
        return EXIT_FAILURE;
    }

    printf("fd is %d\n", fd);

    /* 
     * fd table for this running process: 
     *  0 stdin
     *  1 stdout
     *  2 stderr
     *  3 testfile.txt (O_WRONLY)
     */

    char buffer[20];
    aprintf(buffer, "ABCD%03dEFGH", fd);
    int rc = write(fd, buffer, strlen(buffer));
    printf("Wrote %d bytes to fd %d\n", rc, fd);

    int important = 32768;
    rc = write(fd, &important, sizeof(int));
    printf("Wrote %d bytes to fd %d\n", rc, fd);

    short q = 0xfade;
    rc = write(fd, &q, sizeof(short));
    printf("Wrote %d bytes to fd %d\n", rc, fd);


    student_t s1;
    s1.rin = 660000001;
    strcopy(s1.rcsid, "ruedam");
    rc = write(fd, &s1, sizeof(student_t));
    printf("Wrote %d bytes to fd %d\n", rc, fd);

    //What is in the stdout buffer at this point...?
    fflush(stdout);

#if 1
    close( fd );
#endif

    return EXIT_SUCCESS;
}