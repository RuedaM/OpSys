/* lecex2-q2.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>


char* file_read(char* filename);



/*
write code to create a child process, then have this child process also call fork(), thereby creating a “grandchild” process.
If any errors occur, display a meaningful error message to stderr and return -1 as a special sentinel value.
*/
int main(int argc, char** argv){
    char* filename = *(argv+argc-1);

    /* PARENT-CHILD PROCESS FORKING */
    pid_t p = fork();
    if (p==-1){
        perror("parent-child fork() failed\n");
        return -1;
    }

    if (p == 0){
        /* CHILD-GRANDCHILD PROCESS FORKING */
        pid_t pp = fork();
        if (pp==-1){
            perror("child-grandchild fork() failed\n");
            return -1;
        }

        

        /* GRANDCHILD PROCESS */
        // The grandchild process must open the input file given as the first command-line argument, i.e., *(argv+1),
        // and count the number of properly nested pairs of '{' and '}' characters in the file.
        // After processing the input file, the grandchild process returns the accumulated count as its exit status.
        if (pp==0){
            //printf("==Entered GRANDCHILD Process==\n");

            char* buffer = file_read(filename);
            if(buffer==NULL) {return -1;}

            char* ptr = buffer;
            int count = 0;
            int depth = 0;

            while(*ptr){
                if(*ptr=='{'){
                    depth++;  // Increase nesting depth
                } 
                else if(*ptr=='}'){
                    if(depth>0){  
                        count++;  // Found a valid pair
                        depth--;  // Close the last opened '{'
                    }
                }

                ptr++;
            }
            free(buffer);

            if (count>=128){
                perror("ERR: past max return value\n");
                abort();
            }

            if(count==1){
                printf("GRANDCHILD: counted %u properly nested pair of curly brackets\n", count);
            }
            else{
                printf("GRANDCHILD: counted %u properly nested pairs of curly brackets\n", count);
            }
            
            return count;
        }

        /* CHILD PROCESS */
        // The child process doubles this count and returns it (to the original parent) as its exit status.
        else{
            //printf("==Entered CHILD Process==\n");
            
            /* wait for grandchild process to complete/terminate */
            int sstatus;
            waitpid(pp, &sstatus, 0);   /* BLOCKING */

            if (WIFSIGNALED(sstatus)){
                fprintf(stderr, "CHILD: Grandchild process terminated abnormally (killed by a signal)\n");
                abort();
            }
            else if (WIFEXITED(sstatus)){
                int exit_sstatus = WEXITSTATUS(sstatus);
                
                if (exit_sstatus==-1){
                    fprintf(stderr, "CHILD: rcvd -1 (error)\n");
                    return -1;
                }
                
                int ret = exit_sstatus*2;
                printf("CHILD: doubled %d; returning %d\n", exit_sstatus, ret);
                return ret;
            }
        }
    }

    /* PARENT PROCESS */
    // Finally, the original (top-level parent) process outputs this returned value.
    else{
        //printf("==Entered PARENT Process==\n");
        
        int status;
        waitpid(p, &status, 0);   /* BLOCKING */

        if (WIFSIGNALED(status)){
            fprintf(stderr, "PARENT: Child process terminated abnormally (killed by a signal)\n");
            return -1;
        }
        else if (WIFEXITED(status)){
            int exit_status = WEXITSTATUS(status);

            if (exit_status==-1){
                fprintf(stderr, "PARENT: rcvd -1 (error)\n");
                return -1;
            }

            printf("PARENT: there are %d properly nested curly brackets\n", exit_status);
        }
    }
}





char* file_read(char* filename){
    int fd = open(filename, O_RDONLY); // read only
    if (fd == -1){
        fprintf(stderr, "GRANDCHILD: open() failed: No such file or directory\n");
        return NULL;
    }

    char* ret;
    
    // Get entire file size
    int fileSize = lseek(fd, 0, SEEK_END);
    // Reset file ptr
    lseek(fd, 0, SEEK_SET);
    // Use ret
    ret = calloc(fileSize+1, sizeof(char));
    int rc = read(fd, ret, fileSize);
    *(ret+rc) = '\0';

    close(fd);

    return ret;
}