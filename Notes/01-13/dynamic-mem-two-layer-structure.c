/* dynamic-mem-two-layer-structure.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    //array of strings, i.e., array of char arrays
    char** names;   //array of char*, same as char* name[]

#if 0
    names = malloc(51 * seizeof(char*));   //uninitialized here
#endif
    names = calloc(51, sizeof(char*));

    *(names+2) = calloc(7, sizeof(char));   // or malloc()...
    strcpy( *(names+2), "Lakers" );
    printf("Let's go, %s!\n", *(names+2));

#if 0
    // Either of these wil cause a memoy leak: data structure left floating in help if ptr is arubtly "cut off" from it
    *(names+2) = NULL;
    *(names+2) = calloc(16, sizeof(char));
#endif

    //Use realloc() to expand the size of names[2]
    realloc( *(names+2), 16*sizeof(char) );
    //Note, you don't really need ^ this *sizeof(char) part, but its good to be safe + clear about what you are doing

    strcat( *(names + 2), " in 2025" );
    printf( "Let's go, %s!\n", *(names + 2) );
    
    //But what is the exact output?
    printf("==> %c\n", names[2][4]);
    printf("==> %c\n", *(*(names+2)+4));

    free(*(names+2));
    free(names);

    return EXIT_SUCCESS;
}

#if 0
                  char *
                 +------+
  names ---> [0] | NULL |
                 +------+
             [1] | NULL |      [0] [1] [2] [3] [4] [5] [6]
                 +------+     +---+---+---+---+---+---+----+
             [2] |  ========> | L | a | k | e | r | s | \0 |
                 +------+     +---+---+---+---+---+---+----+
             [3] | NULL |
                 +------+
                   ...
                 +------+
            [50] | NULL |
                 +------+
#endif
