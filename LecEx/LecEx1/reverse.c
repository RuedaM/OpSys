/* reverse.c

COMPILE: gcc -Wall -Werror -g reverse.c
RUN: [valgrind] ./a.out
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "reverse.h"



int main(){
    char* ptr;
    char word[6] = "Hello";
    ptr = word;
    printf("Normal: %s\n", ptr);

    char* ptr2 = reverse(ptr);
    printf("Reverse: %s\n", ptr2);
}