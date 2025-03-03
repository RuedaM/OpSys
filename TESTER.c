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



int main(){
    char var = 64;
    printf("%d", var);
    return EXIT_SUCCESS;
}