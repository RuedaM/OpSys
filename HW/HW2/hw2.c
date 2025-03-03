/* hw2.c */

/*
 * TERMINAL INPUT:
 * gcc -Wall -Werror -Wextra -g -o hw2.out hw2.c
 * gcc -Wall -Werror -Wextra -g -o hw2.out -D DEBUG_MODE hw2.c
 * gcc -Wall -Werror -Wextra -g -o hw2.out -D NO_PARALLEL hw2.c     <=== REQUIRED
 * valgrind -s ./hw2.out
 * 3 3 0 0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>

int move(char **board, int m, int n, int r, int c, int rNew, int cNew, int* pfd);
int number_of_visited_squares(char **board, int m, int n);
// int is_full_tour(char** board, int m, int n);

int rOrigin;
int cOrigin;

int main(int argc, char **argv){
    // #ifdef DEBUG_MODE
    //     for(int i=1 ; i<=4 ; i++){
    //         printf("Arg #%d: %s // ", i, *(argv+i));
    //     }
    //     printf("\n");
    // #endif
    setbuf(stdout, NULL);

    // Command line input
    if (argc!=5){ // (1 .out file+4 numbers = 5 args)
        perror("ERR: invalid number of arguments");
        perror("USAGE: ./hw2.out <boardRows> <boardCols> <startRow> <startCol>");
        return EXIT_FAILURE;
    }

    int m = atoi(*(argv+1)); // board row count
    int n = atoi(*(argv+2)); // board col count
    int r = atoi(*(argv+3)); // starting coords row
    int c = atoi(*(argv+4)); // starting coords col

    if (m<0 || n<0){
        perror("ERR: invalid board size");
        perror("USAGE: boardRows > 0");
        perror("USAGE: boardCols > 0");
        return EXIT_FAILURE;
    }
    else if ((r<0 || r>m) || (c<0 || c>n)){
        perror("ERR: invalid start location");
        perror("USAGE: 0 < startRow < boardRows");
        perror("USAGE: 0 < startCol < boardCols");
        return EXIT_FAILURE;
    }

    // Local variables
    int closes = 0;
    int opens = 0;

    // Get top-level PID
    pid_t parentPID = getpid();
    // printf("Parent PID: %d\n", parentPID);

    // Create board
    char **board = calloc(m, sizeof(char*));
    if (!board){ perror("calloc() failed"); return EXIT_FAILURE; }
    for(int i=0 ; i<m ; i++){
        *(board+i) = calloc(n, sizeof(char)); // Remeber: calloc() initializes to int 0
        if (!(*(board+i))){ perror("calloc() failed"); return EXIT_FAILURE; }
    }
    printf("PID %d: Solving the knight's tour problem for a %dx%d board\n", getpid(), m, n);

    // Create pipe
    int* pipefd = calloc(2, sizeof(int));
    int rc = pipe(pipefd);
    if (rc==-1){ perror("pipe() failed"); return EXIT_FAILURE; }

    // Place piece at init. position; RECURSIVELY check all possible moves
    printf("PID %d: Starting at row %d and column %d (move #%d)\n", getpid(), r, c, 1);
    int deadEndStatus = move(board, m, n, r, c, r, c, pipefd);
    close(*(pipefd+1));

    // Collect end-of-tree information
    printf("PID %d: ", getpid());
    printf("Search complete; ");

    if (getpid()==parentPID){
        //printf("MADE IT\n");

        char *buffer = calloc(2, sizeof(char));
        while(read(*(pipefd+0), buffer, sizeof(char)) > 0){
            if (*(buffer)=='C'){ closes++; }
            if (*(buffer)=='O') { opens++; }
        }
        free(buffer);

        //printf("MADE IT TOO\n");

        if (closes>0 || opens>0){ printf("found %d open tours and %d closed tours\n", opens, closes); }
        else{ printf("best solution(s) visited %d squares out of %d\n", deadEndStatus, m * n); }
    }

    // Close read portion of pipe
    close(*(pipefd+0)); close(*(pipefd+1));
    
    // Free dynamically-allocated memory
    for (int i=0; i<m; i++){ free(*(board+i)); }
    free(board);
    free(pipefd);

    return EXIT_SUCCESS;
}




int move(char** board, int m, int n, int r, int c, int rNew, int cNew, int* pfd){
    int pipeWrite = *(pfd+1);
    int exitStatus = 0;

    // Make the move on the board
    *(*(board+rNew)+cNew) = 'X';
    // Update r and c to reflect current position
    r = rNew; c = cNew;
#ifdef DEBUG_MODE
    printf("PID %d: ===Move #%d made===\n", getpid(), number_of_visited_squares(board, m, n));
#endif

    int** possCoords = calloc(8, sizeof(int*)); // Array for holding all possible moves
    int pCCount = 0;
    if (r+2<=(m-1) && c+1<=(n-1) && *(*(board+r+2)+c+1)!='X'){ // Up-Right
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = r+2;
        *(*(possCoords+pCCount)+1) = c+1;
        pCCount++;
        // #ifdef DEBUG_MODE
        //         printf("PID %d: Up-Right possible\n", getpid());
        // #endif
    }
    if (r+1<=(m-1) && c+2<=(n-1) && *(*(board+r+1)+c+2)!='X'){ // Right-Up
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = r+1;
        *(*(possCoords+pCCount)+1) = c+2;
        pCCount++;
        // #ifdef DEBUG_MODE
        //         printf("PID %d: Right-Up possible\n", getpid());
        // #endif
    }
    if (r-1>=0 && c+2<=(n-1) && *(*(board+r-1)+c+2)!='X'){ // Right-Down
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = r-1;
        *(*(possCoords+pCCount)+1) = c+2;
        pCCount++;
        // #ifdef DEBUG_MODE
        //         printf("PID %d: Right-Down possible\n", getpid());
        // #endif
    }
    if (r-2>=0 && c+1<=(n-1) && *(*(board+r-2)+c+1)!='X'){ // Down-Right
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = r-2;
        *(*(possCoords+pCCount)+1) = c+1;
        pCCount++;
        // #ifdef DEBUG_MODE
        //         printf("PID %d: Down-Right possible\n", getpid());
        // #endif
    }
    if (r-2>=0 && c-1>=0 && *(*(board+r-2)+c-1)!='X'){ // Down-Left
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = r-2;
        *(*(possCoords+pCCount)+1) = c-1;
        pCCount++;
        // #ifdef DEBUG_MODE
        //         printf("PID %d: Down-Left possible\n", getpid());
        // #endif
    }
    if (r-1>=0 && c-2>=0 && *(*(board+r-1)+c-2)!='X'){ // Left-Down
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = r-1;
        *(*(possCoords+pCCount)+1) = c-2;
        pCCount++;
        // #ifdef DEBUG_MODE
        //         printf("PID %d: Left-Down possible\n", getpid());
        // #endif
    }
    if (r+1<=(m-1) && c-2>=0 && *(*(board+r+1)+c-2)!='X'){ // Left-Up
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = r+1;
        *(*(possCoords+pCCount)+1) = c-2;
        pCCount++;
        // #ifdef DEBUG_MODE
        //         printf("PID %d: Left-Up possible\n", getpid());
        // #endif
    }
    if (r+2<=(m-1) && c-1>=0 && *(*(board+r+2)+c-1)!='X'){ // Up-Left
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = r+2;
        *(*(possCoords+pCCount)+1) = c-1;
        pCCount++;
        // #ifdef DEBUG_MODE
        //         printf("PID %d: Up-Left possible\n", getpid());
        // #endif
    }
    // #ifdef DEBUG_MODE
    //     printf("PID %d: pCCount: %d\n", getpid(), pCCount);
    //     for (int i=0 ; i<pCCount ; i++){
    //         printf("Option %d: (%d,%d)\n", i+1, *(*(possCoords+i)+0), *(*(possCoords+i)+1));
    //     }
// #endif



    // No more possible moves: dead end OR full tour
    if (pCCount == 0){
#ifdef DEBUG_MODE
        printf("PID %d: No more possible moves...\n", getpid());
#endif
        int nVS = number_of_visited_squares(board, m, n);
        
        
        if (((m*n)-nVS)>=1){ // DEAD END: returns the number of squares covered
#ifndef QUIET
            printf("PID %d: Dead end at move #%d\n", getpid(), nVS);
#endif
            exitStatus = nVS;
        }
        
        else{ // FULL TOUR
            if ((((r-2) == rOrigin) && ((c-1) == cOrigin)) ||
                (((r-2) == rOrigin) && ((c+1) == cOrigin)) ||
                (((r+2) == rOrigin) && ((c-1) == cOrigin)) ||
                (((r+2) == rOrigin) && ((c+1) == cOrigin)) ||
                (((r+1) == rOrigin) && ((c-2) == cOrigin)) ||
                (((r-1) == rOrigin) && ((c-2) == cOrigin)) ||
                (((r+1) == rOrigin) && ((c+2) == cOrigin)) ||
                (((r-1) == rOrigin) && ((c+2) == cOrigin)))
            { // CLOSED KNIGHT'S TOUR
#ifndef QUIET
                printf("PID %d: Found an closed knight's tour; notifying top-level parent\n", getpid());
#endif
                write(pipeWrite, "C", sizeof(char));
            }
            else
            { // OPEN KNIGHT'S TOUR
#ifndef QUIET
                printf("PID %d: Found an open knight's tour; notifying top-level parent\n", getpid());
#endif
                write(pipeWrite, "O", sizeof(char));
            }

            exitStatus = (m*n); // Should have visited all squares => return m*n
        }
    }


    // If only one possible move to make: make the move
    else if (pCCount==1){
        rNew = *(*(possCoords+0)+0);
        cNew = *(*(possCoords+0)+1);
        exitStatus = move(board, m, n, r, c, rNew, cNew, pfd);
    }

    
    // If multiple possible moves to make: forking for-loop
    else{
#ifndef QUIET
        printf("PID %d: %d possible moves after move #%d; creating %d child processes...\n", getpid(), pCCount, number_of_visited_squares(board, m, n), pCCount);
#endif

        int maxMoves = 0; // Max number of moves to an end condition (dead end or full tour)
        int status, eStatus;

        for (int i=0 ; i<pCCount ; i++){
#ifdef DEBUG_MODE
            printf("PID %d: In loop, forking at (%d,%d)...\n", getpid(), *(*(possCoords+i)+0), *(*(possCoords+i)+1));
#endif
            rNew = *(*(possCoords+i)+0);
            cNew = *(*(possCoords+i)+1);

            pid_t p = fork();
            if (p==-1){ perror("fork() failed"); return EXIT_FAILURE; }

            if (p==0) {   // CHILD PROCESS
                status = move(board, m, n, r, c, rNew, cNew, pfd);   // (recycling the "status" variable since I'm lazy)
                
                close(pipeWrite);
                for (int i=0 ; i<8 ; i++){ free(*(possCoords+i)); }
                free(possCoords);
                for (int i=0; i<m; i++){ free(*(board+i)); }
                free(board);
                free(pfd);

                exit(status); // Preemptively exit to find max val in parent process
                // exitStatus = status;
            }

            else{   // PARENT PROCESS
#ifdef NO_PARALLEL // CONCURRENT: Block at each child's creation, find the maxMoves iteratively
#ifdef DEBUG_MODE
                printf("=CONCURRENT: BLOCKING FOR THIS CHILD=\n");
#endif
                waitpid(p, &status, 0); // BLOCKING BEHAVIOR
                if (WIFSIGNALED(status)){
                    fprintf(stderr, "child process terminated abnormally (killed by a signal)\n");
                    abort();
                }else if (WIFEXITED(status)){
                    eStatus = WEXITSTATUS(status);
                    if (maxMoves<eStatus){ maxMoves=eStatus; }
                }
#endif
            }
        }
#ifndef NO_PARALLEL // PARALLEL: collect all child process after the for-loop, find maxMoves summatively
        while (waitpid(-1, &status, 0) > 0) { // -1 = wait for ANY child process, regardless of order ; waitpid returns 0 if no child process in queue
#ifdef DEBUG_MODE
            printf("=PARALELL: WORKING THROUGH ALL CHILDREN NOW=\n");
#endif
            if (WIFSIGNALED(status)){
                fprintf(stderr, "child process terminated abnormally (killed by a signal)\n");
                abort();
            }
            else if (WIFEXITED(status)){
                eStatus = WEXITSTATUS(status);
                if (maxMoves<eStatus){ maxMoves=eStatus; }
            }
        }
#endif
        exitStatus = maxMoves;
    }



    // Free dynamically-allocated memory
    for (int i=0 ; i<8 ; i++){ free(*(possCoords+i)); }
    free(possCoords);

    // Close pipe
    close(pipeWrite);

    // printf("PID %d: Exiting w/ status: %d\n", getpid(), exitStatus);
    return exitStatus;
}



int number_of_visited_squares(char **board, int m, int n){
    int ret = 0;
    for (int i=0; i<m ; i++){
        for (int j=0; j<n ; j++){
            if (*(*(board+i)+j)=='X'){ ret++; }
        }
    }
    return ret;
}

// int is_full_tour(char** board, int m, int n){
//     if (number_of_visited_squares(board, m, n) == m*n){ return 1; }
//     else{ return 0; }
// }