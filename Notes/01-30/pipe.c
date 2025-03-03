/* pipe.c */

/*
 * "pipe" = unidirectional communication channel -- man 2 pipe
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



int main(){

    int pipefd[2];   // Array to hold the two pipe (file) descriptors:
                        // pipefd[0] = "read" end of the pipe
                        // pipefd[1] = "write" end of the pipe
                        // Of course, in practice, this should e dynamically allocated (calloc())

    // Create pipe
    int rc = pipe(pipefd);
    if (rc==-1){ perror("pipe() failed"); return EXIT_FAILURE; }

  /* fd table:
   *
   *  0: stdin
   *  1: stdout
   *  2: stderr                  +--------+
   *  3: pipefd[0] <====READ=====| buffer | think of this buffer as a
   *  4: pipefd[1] =====WRITE===>| buffer |  temporary hidden "file"
   *                             +--------+
   */

    printf("Created pipe: pipefd[0] is %d and pipefd[1] is %d\n", pipefd[0],pipefd[1]);
    

    // Write data to the pipe
    int bytes_written = write(pipefd[1], "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    printf("Wrote %d bytes to the pipe\n", bytes_written);

    // Read data from pipe
    char buffer[20];
    int bytes_read = read(pipefd[0], buffer, 10);   // "only read in 10 bytes"
    buffer[bytes_read] = '\0';
    printf("Read %d bytes: %s\n", bytes_read, buffer);

    int bytes_read = read(pipefd[0], buffer, 10);   // "only read in the next 10 bytes"
    buffer[bytes_read] = '\0';
    printf("Read %d bytes: %s\n", bytes_read, buffer);

    int bytes_read = read(pipefd[0], buffer, 10);   // "only read in the next 10 bytes"
    buffer[bytes_read] = '\0';
    printf("Read %d bytes: %s\n", bytes_read, buffer);   // but only 6 chars left to read

#if 0
    // if you were to ignore the close() above:
    // At this point, the pipe is empty...this next read() call would BLOCK INDEFINITELY since:
    //  A) there is no data to read
    //  B) there is at least one active/open "write" descriptor on the pipe
    int bytes_read = read(pipefd[0], buffer, 10);   // "only read in 10 bytes"
    buffer[bytes_read] = '\0';
    printf("Read %d bytes: %s\n", bytes_read, buffer);
#endif

    close(pipefd[1]);   // Close the "write" end of the pipe

  /* fd table:
   *
   *  0: stdin
   *  1: stdout
   *  2: stderr                  +--------+
   *  3: pipefd[0] <====READ=====| buffer | think of this buffer as a
   *  4:                         | buffer |  temporary hidden "file"
   *                             +--------+
   */


    int bytes_read = read(pipefd[0], buffer, 10);
    printf("bytes_read is %d\n", bytes_read);

    int bytes_read = read(pipefd[0], buffer, 10);
    printf("bytes_read is %d\n", bytes_read);

    int bytes_read = read(pipefd[0], buffer, 10);
    printf("bytes_read is %d\n", bytes_read);

    close(pipefd[0]);   // Close the "read" end of the pipe

  /* fd table:
   *
   *  0: stdin
   *  1: stdout
   *  2: stderr
   *  3:
   *  4:
   */

    int bytes_read = read(pipefd[0], buffer, 10);
    printf("bytes_read is %d\n", bytes_read);

    return EXIT_SUCCESS;
}