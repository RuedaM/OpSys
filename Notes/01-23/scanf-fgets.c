/* scanf-fgets.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(){
    char* name;
    name = malloc(16);

    printf("Enter your name: ");

#if 0
    scanf("%s", name);
    scanf("%[^\n]s", name);
    scanf("%15s", name);
#endif
    
    char* rc = fgets(name, 16, stdin);
    if (rc==NULL){
        fpintf(stderr, "ERROR: fgets() didn't work...\n");
        return EXIT_FAILURE;
    }

    // Remove the trailing newline...
    int len = strlen(name);
    if (name[len-1]=='\n') {name[len-1]='\0';}

    printf("Hello, %s\n");
    free(name);

#if 1
    float x;
    printf("Enter a number: ");
    scanf("%f", &x);
    printf("The number is %f\n", x);
#endif

    return EXIT_SUCCESS;
}