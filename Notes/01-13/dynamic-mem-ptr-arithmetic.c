/* dynamic-mem-ptr-arithmetic.c */

/* TO DO: calculate how much memory is allocated on the stack
 *         for static allocations
 *
 *        calculate how much memory is allocated on the heap
 *         for dynamic allocations
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    //Dynamically allocate 100 bytes on the runtime heap
    char* s = malloc(100);

    //dynamically allocate 100 ytes on the runtime heap...?
    char* t = calloc(100, sizeof(char));
        //calloc() ibe all zero bytes, i.e., the '\0' char

    printf("\"%s\" \"%s\"\n", s, t);

    s[0] = 'A'; s[1] = 'B'; s[2] = 'C'; s[3] = 'D'; s[4] = '\0';
    s[20] = 'X'; s[21] = 'Y'; s[22] = 'Z'; s[23] = '\0';

    printf("\"%s\" \"%s\"\n", s, t);

    printf("\"%s\" \"%s\"\n", s[20], t);
    printf("\"%s\" \"%s\"\n", &s[20], t);
    printf("\"%s\" \"%s\"\n", s+20, t);   //move 20 bytes down the array
    printf("\"%s\" \"%s\"\n", s+22, t);
    printf("\"%s\" \"%s\"\n", s, t);

    free(s);
    free(t);

#if 0
    int numbers[50];   //static allocation
#endif

    // dynamically allocated memory for an array of integers of size 50 on the runtime heap...
    // (all initialized to 0)
    int* numbers = calloc(50, sizeof(int));  //50units * 4bytes per int = 200 total bytes allocated on runtime heap

    numbers[18] = 1234;
    *(numbers+18) = 12345;
    // Note that numbers[18] and *(numbers+18) are equivalent: *(numbers+18) = "dereference the value 18 bytes out from the start of 'numbers'"
    // compiler does: numbers + (18 x sizeof(int)) == numbers+72
    // ^^^ NEVER MAKE THIS CALCULATION IN CODE, LET THE COMPILER WORRY ABOUT THAT

    printf("%d\n", numbers[18]);
    printf("%d\n", *(numbers+18));   // same as above
    printf("%d\n", *(numbers+48));   // displays zero bytes
#if 0
    printf("%d\n", *(numbers+480));   // displays zero bytes again
    printf("%d\n", *(numbers+4800));   // and again
    printf("%d\n", *(numbers+48000));   // finally a seg fault
#endif

    free(numbers);

    return EXIT_SUCCESS;
}