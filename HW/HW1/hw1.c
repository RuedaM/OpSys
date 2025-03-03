/* hw1.c */

/*
 * In this first homework, you will use C to implement a rudimentary cache of words, which will
 * be populated with strings read from one or more input files. Your cache must be a dynamically
 * allocated hash table of a given fixed size that handles collisions by simply replacing the existing
 * word.
 * 
 * As part of this, you are required to provide a library function (in a hw1.h header file) that
 * “tokenizes” an input string to dynamically identify each valid word.
 * 
 * This hash table is really just a one-dimensional array of char * pointers. These pointers should all
 * initially be set to NULL, then set to point to dynamically allocated strings for each cached word.
 * 
 * Do not use malloc() or memset() in your code.
 * 
 * COMPILE: gcc -Wall -Werror -g hw1.c
 * RUN: <valgrind> ./a.out <#> <filename>.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#include "hw1.h"



int hash(char* string, int cacheSize);

char** ptr;



int main(int argc, char** argv){
    
    int cacheSize = atoi(*(argv+1));   // terminal input = char, so convert
    if (cacheSize<0){
        fprintf(stderr, "ERROR: Invalid argument -- cache size must be a positive value\n");
        return EXIT_FAILURE;
    }

    //printf("Creating cache...\n");
    char** cache = calloc(cacheSize, sizeof(char*));
    ptr = cache;
    while(ptr!=NULL && *ptr!=NULL){
        *ptr = NULL;
        ptr++;
    }
    //printf("Created\n");

    //char* string = "The 'quick brown fox,' jumps, over the lazy, dog\0";
    //char* string = "RPI) is a private research university in Troy, New York, United Sta";
    //char* string = "a a a a a a a a a a a a a a a a a a a a a a a a the";
    //char* string = "      /.,/;][])(                  the";
    //char* string = "the";
    //char* string = "La La Land";
    //char* string = "LITTLE \"FRIENDS\" MAY PROVE GREAT \"FRIENDS.\"\n";
    //char* string = "Once when a Lion was asleep a little Mouse began running up and down upon him;";
    //char* string =  "a.b.c.d.e.f.g.h.i.j.k.l.mn.o.p.q.r.s.t.u.v.w.x.y.z";
    char* string = calloc(1, sizeof(char));

    //printf("argc: %d\n", argc);
    for (int i=2 ; i<=argc-1 ; i++){
        //printf("Looking at file: %s\n", *(argv+i));

        char* filename = *(argv+i);
        
        int fd = open(filename, O_RDONLY);
        if (fd==-1){
            perror( "open() failed" );
            return EXIT_FAILURE;
        }
        
        // Get entire file size
        int fileSize = lseek(fd, 0, SEEK_END);
        // Reset file ptr
        lseek(fd, 0, SEEK_SET);
        // Create+Use readBuffer
        char* readBuffer = calloc(fileSize+1, sizeof(char));
        int rc = read(fd, readBuffer, fileSize);
        *(readBuffer+rc) = '\0';

        string = realloc(string, (strlen(string)+fileSize+1)*sizeof(char));
        if (string==NULL){
            perror("realloc failed");
            free(string);
            return EXIT_FAILURE;
        }
        
        strcat(string, readBuffer);
        //*(string+strlen(string)+fileSize+1) = '\0';
        //printf(":::STRING:::\n%s\n", string);

        free(readBuffer);

        //printf("Loaded to string\n");
        close(fd);

    }
    //printf("Done reading files\n");

    char** tokenized = tokenize(string);
    if (tokenized==NULL){
        perror("realloc failed");
        free(string);
        return EXIT_FAILURE;
    }
    
#ifdef DEBUG
    printf("Tokenized string:\n");
    ptr = tokenized;
    while (*ptr!=NULL){
        printf("%s \\0 ", *ptr);
        ptr++;
    } printf("\n");
#endif

    //printf("hashing...\n");
    ptr = tokenized;
    while (*ptr!=NULL){
        //printf("OPERATING ON: %s %ld\n", *ptr, strlen(*ptr));
        int hashVal = hash(*ptr, cacheSize);
        char* op;

        if (*(cache+hashVal) == NULL){
            op = "calloc";
            *(cache+hashVal) = calloc(strlen(*ptr)+1, sizeof(char));
            for (unsigned int i=0 ; i<strlen(*ptr)+1 ; i++){
                *(*(cache+hashVal)+i) = tolower(*(*ptr+i));
                if (i==strlen(*ptr)){
                    *(*(cache+hashVal)+i) = '\0';
                }
            }
        }
        else{
            if (strlen(*ptr)!=strlen(*(cache+hashVal))){
                op = "realloc";
                *(cache+hashVal) = realloc(*(cache+hashVal), (strlen(*ptr)+1)*sizeof(char));
                for (unsigned int i=0 ; i<strlen(*ptr)+1 ; i++){
                    *(*(cache+hashVal)+i) = tolower(*(*ptr+i));
                    if (i==strlen(*ptr)){
                        *(*(cache+hashVal)+i) = '\0';
                    }
                }
            }
            else {
                op = "nop";
                for (unsigned int i=0 ; i<strlen(*ptr)+1 ; i++){
                    *(*(cache+hashVal)+i) = tolower(*(*ptr+i));
                    if (i==strlen(*ptr)){
                        *(*(cache+hashVal)+i) = '\0';
                    }
                }
            }
        }

        printf("Word \"%s\" ==> %d (%s)\n", *ptr, hashVal, op);
        //printf("%s", op);

        ptr++;
    }
    //printf("hashed\n");
    printf("\n");


    // Print the non-NULL cache
    char** temp = cache;
    printf("Cache:\n");
    for (int i=0 ; i<cacheSize ; i++){
        if (*temp!=NULL) {printf("%c%d%c ==> \"%s\"\n", 91, i, 93, *temp);}
        temp++;
    }


    // Freeing memory
    char** ptrFree = tokenized;
    while (*ptrFree!=NULL) {
        free(*ptrFree);
        ptrFree++;
    }
    free(tokenized);
    
    for (int i=0 ; i<cacheSize ; i++){
        if (*(cache+i)!=NULL) {
            free(*(cache+i));
            *(cache+i) = NULL;
        }
    }
    free(cache);

    free(string);
}





int hash(char* string, int cacheSize){
    int sum = 0;
    char* ptr = string;
    while (ptr!=NULL && *ptr!='\0') {
        sum+=tolower(*ptr);
        ptr++;
    }
    return (sum % cacheSize);
}