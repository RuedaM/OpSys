/* chunk.c

Use open(), read(), lseek(), and close() to extract n-byte chunks from filename, skipping every other chunk by using lseek().
Refer to the man pages to better understand these functions. Note that you must use lseek() to skip over bytes in the file.
Display every other chunk to stdout, delimiting them with a pipe '|' character.
Only display the last chunk if it is of size n.
Always append one newline '\n' character at the end of your program execution.
As a hint, use atoi() to convert a string into an integer.
Use the return value of read() to determine when to stop.


COMPILE: gcc -Wall -Werror -g chunk.c
RUN: [valgrind] ./a.out [#] [filename].txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



int main(int argc, char** argv){
    // Get and store inputs
    int chunkSize = atoi(*(argv+1));   // terminal input = char, so convert
    char* filename = *(argv+2);

    // fd = "file descriptor"
    int fd = open(filename, O_RDONLY);   // read only
    if(fd==-1){
        perror( "open() failed" );
        return EXIT_FAILURE;
    }

    char* buffer = (char*) malloc(chunkSize);   // Dynamically allocate runtime heap space for string

    // Perform the first chunk read and chunk skip
    int bytesReadPre = read(fd, buffer, chunkSize);
    if (bytesReadPre<chunkSize){
        printf("\n");
        return EXIT_SUCCESS;
    }
    else{
        printf("%s", buffer);
        lseek(fd, chunkSize, SEEK_CUR);
    }
    
    // Continue until the end is reached, this time adding pipes (|) between chunks
    while(1){
        int bytesRead = read(fd, buffer, chunkSize);

        if (bytesRead<chunkSize){break;}
        printf("|%s", buffer);
        lseek(fd, chunkSize, SEEK_CUR);
    }

    printf("\n");

    // Close the file, free memory
    close (fd);
    free(buffer);

    return EXIT_SUCCESS;
}