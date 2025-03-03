/* lecex2-q1.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

/*
Open and read from a file called lecex2.txt.
Using open() and read(), read the nth character in that file, then close the file.
If all is successful, exit the child process and return the nth character (i.e., one byte) as its exit status.
If an error occurs, display an error message to stderr and use the abort() library function to abnormally terminate the child process.
*/
int lecex2_child(unsigned int n)
{
    //printf("==Entered lecex2_child==\n");

    int ret;

    // fd = "file descriptor"
    int fd = open("lecex2.txt", O_RDONLY); // read only
    if(fd==-1){
        fprintf(stderr, "ERR: open() failed\n");
        abort();
    }

    // Check if fileSize < n
    unsigned int fileSize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    if(fileSize<n){
      fprintf(stderr, "ERR: read() failed\n");
      abort();
    }


    //printf("Getting character...\n");
    // Create + Use fileBuffer: read entire file to buffer
    char *fileChar = calloc(2, sizeof(char));
    // Move to nth character in file
    // Note that the first byte in the file is considered n = 1, the second is n = 2, etc.
    lseek(fd, n-1, SEEK_SET);

    int charsRead = read(fd, fileChar, 1);
    if(charsRead==-1){
        fprintf(stderr, "ERR: read() failed\n");
        abort();
    }
    *(fileChar+charsRead) = '\0';
    close(fd);

    ret = *fileChar;
    //printf("Found char: %c\n", ret);

    free(fileChar);

    return ret;
}

/*
Use waitpid() to suspend the parent process and wait for the child process to terminate.
*/
int lecex2_parent()
{
    //printf("==Entered lecex2_parent==\n");
    int status;
    // while (waitpid(-1, &status, 0) > 0){

    // }
    waitpid(-1, &status, 0); /* BLOCKING */

    //printf("PARENT: child process %d terminated...\n", child_pid);

    // Abnormal Termination
    if (WIFSIGNALED(status))
    {
        printf("PARENT: child process terminated abnormally!\n");
        return EXIT_FAILURE;
    }

    // Normal Termination
    else if (WIFEXITED(status))
    {
        int exit_status = WEXITSTATUS(status);
        printf("PARENT: child process exited with status '%c'\n", exit_status);
    }

    return EXIT_SUCCESS;
}