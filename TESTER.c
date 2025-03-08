/* TESTER.c */

/*
 * cd Documents/Academic/RPI/OpSys/
 * gcc -Wall -Werror -Wextra -g TESTER.c
 * ./a.out
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>



int main() {
    const int fileCount = 3; // Number of files to create
    const char* filenames[] = {"file1.txt", "file2.txt", "file3.txt"};
    const char* fileContents[] = {
        "This is the content for file 1.\n",
        "This is the content for file 2.\n",
        "This is the content for file 3.\n"
    };

    int fileDescriptors[fileCount]; // Array of file descriptors

    // Open files and write content
    for (int i = 0; i < fileCount; i++) {
        // Open file with write and create permissions, and set file permissions to 0644
        fileDescriptors[i] = open(filenames[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fileDescriptors[i] == -1) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        // Write content to the file
        ssize_t bytesWritten = write(fileDescriptors[i], fileContents[i], strlen(fileContents[i]));
        if (bytesWritten == -1) {
            perror("Error writing to file");
            close(fileDescriptors[i]);
            exit(EXIT_FAILURE);
        }
        printf("Written to %s\n", filenames[i]);
    }

    // Close all files
    for (int i = 0; i < fileCount; i++) {
        if (close(fileDescriptors[i]) == -1) {
            perror("Error closing file");
            exit(EXIT_FAILURE);
        }
    }

    printf("All files written and closed successfully.\n");
    return 0;
}