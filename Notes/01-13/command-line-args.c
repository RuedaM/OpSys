/* command-line-args.c */

#include <stdio.h>
#include <stdlib.h>

int main( int argc, char** argv ){
    printf("argc is %d\n", argc);   /* argument count */

    if (argc!=4){
        fprintf(stderr, "ERROR: Invalid arguments\n");
        fprintf(stderr, "USAGE: a.out <filename> <x> <y>\n");
        return EXIT_FAILURE;
    }

    printf("argv[0] is %s\n", argv[0]);   // argv + 0 == ./a.out
    printf("argv[0][2] is %c\n", argv[0][2]);   // argv[0] + 2 == a
    printf("argv[1] is %s\n", argv[1]);   // argv + 1 == a.txt
    printf("argv[2] is %s\n", argv[2]);   // argv + 2 == 12
    printf("argv[3] is %s\n", argv[3]);   // argv + 3 == 34
    printf("argv[argc] is %s\n", argv[argc]);   // argv + argc == NULL (end of array, so NULL delimiter)

#if 0
    printf("argv[4] is %s\n", argv[4]);   // argv + 4 == NULL
    printf("argv[5] is %s\n", argv[5]);   // argv + 5 == [...]
    printf("argv[6] is %s\n", argv[6]);   // argv + 6 == [...]
#endif

    // Other ways to display/navigate the command-line arguments:
    for (int i=0 ; *(argv+i)!=NULL ; i++){
        printf("*(argv+%d) is %s\n", i, *(argv+i));
    }

    for (char** ptr = argv ; *ptr ; ptr++){
        printf("next arg is %s\n", *ptr);
    }
    // ptr* ==> ptr = ptr+1 ==> ptr = ptr+1*sizeof(char*)

    return EXIT_SUCCESS;
}

#if 0
These two are equivalent
int main (int argc, char** argv)
int main (int argc, char* argv[])

                 char *
                +------+
  argv ---> [0] |  =======> "./a.out"
                +------+
            [1] |  =======> "a.txt"
                +------+
            [2] |  =======> "12"
                +------+
            [3] |  =======> "34"
                +------+
            [4] | NULL |
                +------+
#endif