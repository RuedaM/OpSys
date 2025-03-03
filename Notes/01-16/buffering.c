/* buffering.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Clearly, the code below will seg-fault. If we were to
 * print-debug this file, we would see that, in this form,
 * no output will be given and the code will simply seg-fault.
 * 
 * When printing to the terminal (shell) via stdout (fd 1),
 * a '\n' character will "flush" the stdout buffer,
 * i.e., output everything that has been stored in the
 * stdout buffer so far -- this is "line-based buffering"
 * 
 * 
 * When we intead put fd 1 to a file
 * (e.g., with the command: bash$ ./a.out > FILE.txt),
 * the '\n' character no longer flushes the stdout buffer
 * -- this is "block-based buffering" (a.k.a. fully buffered)
 * 
 * 
 * A 3rd type of of buffering is non-buffered (unbuffered),
 * which is what is used for stderr (fd 2)
 * 
 * 
 * The "fix" to the code below is adding '\n' characters
 * to the end of any printf() meant for debugging,
 * i.e., the HERE's
 */





int main(){
    printf("HERE0");   //stdout buffer: "HERE0"
    
    int* x = NULL;

    printf("HERE1");   //stdout buffer: "HERE0HERE1"

    fprintf(stderr, "--do we see this stderr output?");
    
    // This will flush the buffer (regardless of buffering)
    fflush(stdout);

    *x = 2345;

    printf("HERE2");

    printf("x points to %d\n", *x);

    printf("HERE3");

    return EXIT_SUCCESS;
}