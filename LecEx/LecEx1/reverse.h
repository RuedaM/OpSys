/* reverse.h */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>



char* reverse(char *s){
    // Dynamically allocating a variable-sized string on runtime heap via malloc()
    char* buffer = (char*) malloc(strlen(s)+1);   // +1 to account for \0
    // Error check if malloc() fails
    if (buffer == NULL){
        perror("malloc() failed");
        return NULL;
    }

    // Copy s's data to new buffer
    strcpy(buffer, s);

    // Create ptrs to buffer and "spare" ch variable
    int l = strlen(buffer);
    char* start = buffer;
    char* end = buffer+l-1;
    char ch;

    // For until halfway through the word,
    //  swap the first and last characters
    for (int i=0 ; i<(l/2) ; i++){
        //printf("Start: %c /// End: %c\n", *start, *end);

        ch = *start;
        *start = *end;
        *end = ch;

        start++; end--;
    }
    //printf("Final Buffer: %s\n", buffer);
    
    // Overwrite s w/ reversed string in buffer
    strcpy(s, buffer);
    //Always free up allocated data
    free(buffer);

    return s;
}



// char *reverseOLD(char *s){
//     char buffer[32];
//     int i, len = strlen(s);
//     for (i = 0; i < len; i++) {buffer[i] = s[len - i - 1];}
//     for (i = 0; i <= len; i++) {s[i] = buffer[i];}
//     return s;
// }