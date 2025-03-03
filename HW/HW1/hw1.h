/* hw1.h */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>





char** tokenize(char* string){
    //printf("==========\n");
    //printf("Verify-- inputted string is: %s\n", string);

    char** ret = NULL;  // Initialize to NULL
    
    char* ptr = string;
    int charCount = 0;
    int wordCount = 0;
    //printf("charCount: %u /// wordCount: %u /// alphCount: %u\n", charCount, wordCount, alphCount);
    int started = 0;

    while (*ptr){
        //printf("%c ", *ptr);

        if (isalpha(*ptr) && !started){
            //printf("START OF WORD ");
            started = !started;
            // Add char* buffer at new latest index of ret
            ret = realloc(ret, (wordCount + 1) * sizeof(char*));
            if (!ret){
                perror("realloc failed");
                return NULL;
            }

            *(ret+wordCount) = calloc(1, sizeof(char));
            
            if (!*(ret+wordCount)){
                perror("calloc failed");
                return NULL;
            }
            // Add char to buffer
            *(*(ret+wordCount)) = *ptr;
            
            charCount = 1;
        }
        
        else if (isalpha(*ptr) && started){
            //printf("WORD CONT'D... ");
            // Resize buffer
            *(ret+wordCount) = realloc(*(ret+wordCount), (charCount+1)*sizeof(char));
            if (!*(ret+wordCount)) {
                perror("realloc failed");
                return NULL;
            }

            // Add char to buffer
            *(*(ret+wordCount)+charCount) = *ptr;
            charCount++;
        }
        
        else if (!isalpha(*ptr) && started){
            //printf("WORD STOP! ");
            // Resize buffer
            *(ret+wordCount) = realloc(*(ret+wordCount), (charCount+1)*sizeof(char));
            if (!*(ret+wordCount)){
                perror("realloc failed");
                return NULL;
            }

            if (charCount>=2){
                //printf("Yes word! ");
                started = !started;
                // Add \0 to buffer
                *(*(ret+wordCount)+charCount) = '\0';
                
                //printf("Is it a word? ");
                char* ptrFinal = *(ret+wordCount);
                int ccFinal = 0;
                while(*ptrFinal!='\0'){
                    if (isalpha(*ptrFinal)) {ccFinal++;}
                    ptrFinal++;
                }
                if (ccFinal>=2){
                    wordCount++;
                    //printf("YES ");
                }
                
            }
            else{
                //printf("Nevermind... ");
                // Add char to buffer
                *(*(ret+wordCount)+charCount) = *ptr;
            }
            charCount = 0;
        }

#ifdef DEBUG
        printf("\n");
        printf("charCount: %u wordCount: %u\n", charCount, wordCount);
        char** sum = ret;
        printf("   SUMMARY:\n");
        for (int i=0 ; i<wordCount ; i++){
            printf("   ret[%d]: ", i);
            printf("%s", *sum);
            printf("\n");
            sum++;
        }
#endif
        
        ptr++;
    }

    // Finalize the last word if still open
    if (started && charCount>=2){
        // Resize buffer
        *(ret+wordCount) = realloc(*(ret+wordCount), (charCount+1)*sizeof(char));
        if (!*(ret+wordCount)){
            perror("realloc failed");
            return NULL;
        }
        // Add \0 to buffer
        *(*(ret+wordCount)+charCount) = '\0';
        wordCount++;
    }

    // Null-terminate the array of strings
    ret = realloc(ret, (wordCount+1)*sizeof(char*));
    if (!ret){
        perror("realloc failed");
        return NULL;
    }
    *(ret+wordCount) = NULL;

    //printf("==========\n");
    return ret;
}





#if 0
char** tokenize(char* string){
    //printf("==========\n");
    //printf("Verify-- inputted string is: %s\n", string);

    char** ret = NULL;  // Initialize to NULL
    
    char* ptr = string;
    int charCount = 0;
    int wordCount = 0;
    //printf("charCount: %u /// wordCount: %u /// alphCount: %u\n", charCount, wordCount, alphCount);
    int started = 0;

    while(1){
        //printf("%c ", *ptr);

        if (*ptr=='\0'){
            //printf("End of string!\n");
            // Resize buffer
            *(ret+wordCount) = realloc(*(ret+wordCount), (charCount+1)*sizeof(char));
            // Add char to buffer
            *(*(ret+wordCount)+charCount) = '\0';
            
            charCount = 0;
            
            //printf("Is it a word? ");
            char* ptrFinal = *(ret+wordCount);
            int ccFinal = 0;
            while(*ptrFinal!='\0'){
                if (isalpha(*ptrFinal)) {ccFinal++;}
                ptrFinal++;
            }
            
            if (ccFinal>=2){
                wordCount++;
                //printf("YES\n");
            }
            else{
                ret = realloc(ret, (wordCount)*sizeof(char*));
                *(ret+wordCount) = '\0';
                //printf("NO\n");

                printf("%s", *(ret+wordCount-1));
                printf("\n\n\n");
            }
            
            break;
        }

        if (isalpha(*ptr) && !started){
            //printf("Start of word! ");
            started = !started;
            // Add char* buffer at new latest index of ret
            ret = realloc(ret, (wordCount+1)*sizeof(char*));
            *(ret+wordCount) = calloc(1, sizeof(char));
//          if (isalpha(*ptr)){
//              alphCount++;
//          }
            // Add char to buffer
            *(*(ret+wordCount)) = *ptr;

            charCount++;
        }

        else if (isalpha(*ptr) && started){
            //printf("Word cont'd... ");
            // Resize buffer
            *(ret+wordCount) = realloc(*(ret+wordCount), (charCount+1)*sizeof(char));
            // Add char to buffer
            *(*(ret+wordCount)+charCount) = *ptr;
//          alphCount++;
            charCount++;
        }

        else if (!isalpha(*ptr) && started){
            // Resize buffer
            *(ret+wordCount) = realloc(*(ret+wordCount), (charCount+1)*sizeof(char));
            if (charCount>=2){
                //printf("Word stop! ");
                started = !started;
                // Add \0 to buffer
                *(*(ret+wordCount)+charCount) = '\0';
                
                //printf("Is it a word? ");
                char* ptrFinal = *(ret+wordCount);
                int ccFinal = 0;
                while(*ptrFinal!='\0'){
                    if (isalpha(*ptrFinal)) {ccFinal++;}
                    ptrFinal++;
                }
                if (ccFinal>=2){
                    wordCount++;
                    //printf("YES ");
                }
                
            }
            else{
                //printf("Nevermind... ");
                // Add char to buffer
                *(*(ret+wordCount)+charCount) = *ptr;
//              charCount++;
            }
//          alphCount = 0;
            charCount = 0;
        }

        ptr++;

        // printf("charCount: %u wordCount: %u\n", charCount, wordCount);
        // char** sum = ret;
        // printf("   SUMMARY:\n");
        // for (int i=0 ; i<wordCount ; i++){
        //     printf("   ret[%d]: ", i);
        //     printf("%s", *sum);
        //     printf("\n");
        //     sum++;
        // }
    }
    
    //printf("==========\n");

    return ret;
}
#endif