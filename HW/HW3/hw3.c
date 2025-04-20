/* hw3.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>

extern long next_thread_number;
extern int max_squares;
extern int total_open_tours;
extern int total_closed_tours;

int rOrigin;
int cOrigin;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
    char** board;
    int m;
    int n;
    
    int r;
    int c;
    int rNew;
    int cNew;
    
    int threadNumber;
    long unsigned threadID;
    long unsigned mainThreadID;
}move_t;


void* moving(void* arguments);
void print_board(char** board, int m, int n);
void print_move(move_t* move);
int number_of_visited_squares(char** board, int m, int n);
move_t move_copy(move_t* orig);





int solve(int argc, char** argv){
    setbuf(stdout, NULL);
    #if DEBUG_MODE
    printf("~~ Checking args: %s, %s, %s, %s\n", *(argv+1),*(argv+2), *(argv+3), *(argv+4));
    #endif

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


    // Create main thread ID for comparison
    pthread_t mainTID = pthread_self();


    // Create board
    char** board = calloc(m, sizeof(char*));
    if (!board){ perror("calloc() for m failed"); return EXIT_FAILURE; }
    for(int i=0 ; i<m ; i++){
        *(board+i) = calloc(n, sizeof(char)); // Remeber: calloc() initializes to int 0
        if (!(*(board+i))){ perror("calloc() for n failed"); return EXIT_FAILURE;}
    }
    #if DEBUG_MODE
    printf("~~ Printing initial board:\n");
    print_board(board, m, n);
    #endif
    printf("MAIN: Solving the knight's tour problem for a %dx%d board\n", m, n);
 

    // Place piece at init. position, establish origin position
    printf("MAIN: Starting at row %d and column %d (move #%d)\n", r, c, 1);
    rOrigin = r; cOrigin = c;


    // Make the first move
    move_t firstMove = {board, m, n, r, c, r, c, 0, mainTID, mainTID};
    moving(&firstMove);

    // Collect end-of-run information
    printf("MAIN: Search complete; ");
    if ((total_open_tours>0 || total_closed_tours>0) && max_squares==m*n) {printf("found %d open tours and %d closed tours\n", total_open_tours, total_closed_tours);}        
    else {printf("best solution(s) visited %d squares out of %d\n", max_squares, m*n);}


    // Freeing dynamically-allocated memory
    for (int i=0; i<m; i++) {free(*(board+i));}
    free(board);

    return EXIT_SUCCESS;
}





void* moving(void* arguments){
    move_t* move = (move_t*)arguments;
    
    int currTID = move->threadNumber;
    // Get current thread ID - create mutex lock to prevent multiple threads from accessing the same memory
    if (pthread_self()!=move->threadID){
        pthread_mutex_lock(&mutex);
        {
            currTID = next_thread_number;
            next_thread_number++;
            move->threadID = pthread_self();
            move->threadNumber = currTID;
        }
        pthread_mutex_unlock(&mutex);
    }

    #if DEBUG_MODE
    printf("~~ ");
    if (pthread_self()==move->mainThreadID) {printf("MAIN");}
    else {printf("T%d", currTID);}
    printf(": Entering moving() to move to (%d,%d)...\n", move->rNew, move->cNew);
    #endif

    // Make the move on the board, update r and c to reflect current position
    *(*(move->board+move->rNew)+move->cNew) = 'X';
    move->r = move->rNew; move->c = move->cNew;
    #if DEBUG_MODE
    printf("~~ ");
    if (pthread_self()==move->mainThreadID) {printf("MAIN");}
    else {printf("T%d", currTID);}
    printf(": Move #%d made\n", number_of_visited_squares(move->board, move->m, move->n));
    #endif


    // Determine all possible moves to make, store in array
    int** possCoords = calloc(8, sizeof(int*)); // Array for holding all possible moves
    int pCCount = 0;

    if (move->r-2>=0 && move->c+1<=(move->n-1) && *(*(move->board+move->r-2)+move->c+1)!='X'){ // Up-Right
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = move->r-2;
        *(*(possCoords+pCCount)+1) = move->c+1;
        pCCount++;
        #if DEBUG_MODE
        printf("~~ T%d: Down-Right possible\n", currTID);
        #endif
    }
    if (move->r-1>=0 && move->c+2<=(move->n-1) && *(*(move->board+move->r-1)+move->c+2)!='X'){ // Right-Up
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = move->r-1;
        *(*(possCoords+pCCount)+1) = move->c+2;
        pCCount++;
        #if DEBUG_MODE
        printf("~~ T%d: Right-Down possible\n", currTID);
        #endif
    }
    if (move->r+1<=(move->m-1) && move->c+2<=(move->n-1) && *(*(move->board+move->r+1)+move->c+2)!='X'){ // Right-Down
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = move->r+1;
        *(*(possCoords+pCCount)+1) = move->c+2;
        pCCount++;
        #if DEBUG_MODE
        printf("~~ T%d: Right-Up possible\n", currTID);
        #endif
    }
    if (move->r+2<=(move->m-1) && move->c+1<=(move->n-1) && *(*(move->board+move->r+2)+move->c+1)!='X'){ // Down-Right
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = move->r+2;
        *(*(possCoords+pCCount)+1) = move->c+1;
        pCCount++;
        #if DEBUG_MODE
        printf("~~ T%d: Up-Right possible\n", currTID);
        #endif
    }
    if (move->r+2<=(move->m-1) && move->c-1>=0 && *(*(move->board+move->r+2)+move->c-1)!='X'){ // Down-Left
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = move->r+2;
        *(*(possCoords+pCCount)+1) = move->c-1;
        pCCount++;
        #if DEBUG_MODE
        printf("~~ T%d: Up-Left possible\n", currTID);
        #endif
    }
    if (move->r+1<=(move->m-1) && move->c-2>=0 && *(*(move->board+move->r+1)+move->c-2)!='X'){ // Left-Down
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = move->r+1;
        *(*(possCoords+pCCount)+1) = move->c-2;
        pCCount++;
        #if DEBUG_MODE
        printf("~~ T%d: Left-Up possible\n", currTID);
        #endif
    }
    if (move->r-1>=0 && move->c-2>=0 && *(*(move->board+move->r-1)+move->c-2)!='X'){ // Left-Up
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = move->r-1;
        *(*(possCoords+pCCount)+1) = move->c-2;
        pCCount++;
        #if DEBUG_MODE
        printf("~~ T%d: Left-Down possible\n", currTID);
        #endif
    }
    if (move->r-2>=0 && move->c-1>=0 && *(*(move->board+move->r-2)+move->c-1)!='X'){ // Up-Left
        *(possCoords+pCCount) = calloc(2, sizeof(int));
        *(*(possCoords+pCCount)+0) = move->r-2;
        *(*(possCoords+pCCount)+1) = move->c-1;
        pCCount++;
        #if DEBUG_MODE
        printf("~~ T%d: Down-Left possible\n", currTID);
        #endif
    }
    
    
    


    // Deciding

    if (pCCount==0){ // If no possible moves to make: determine if at dead end or at open/closed tour, set exit status
        #if DEBUG_MODE
        printf("T%d: No more possible moves...\n", currTID);
        #endif
        int nVS = number_of_visited_squares(move->board, move->m, move->n);
        
        
        if (((move->m*move->n)-nVS)>=1){ // DEAD END: returns the number of squares covered
            #ifndef QUIET
            if (pthread_self()==move->mainThreadID) {printf("MAIN");}
            else {printf("T%d", currTID);}
            printf(": Dead end at move #%d\n", nVS);
            #endif
            
            pthread_mutex_lock(&mutex);
            {
                if (max_squares<nVS) {max_squares = nVS;}
            }
            pthread_mutex_unlock(&mutex);


        }
        
        else{ // FULL TOUR
            if ((((move->r-2) == rOrigin) && ((move->c-1) == cOrigin)) || // CLOSED KNIGHT'S TOUR
                (((move->r-2) == rOrigin) && ((move->c+1) == cOrigin)) ||
                (((move->r+2) == rOrigin) && ((move->c-1) == cOrigin)) ||
                (((move->r+2) == rOrigin) && ((move->c+1) == cOrigin)) ||
                (((move->r+1) == rOrigin) && ((move->c-2) == cOrigin)) ||
                (((move->r-1) == rOrigin) && ((move->c-2) == cOrigin)) ||
                (((move->r+1) == rOrigin) && ((move->c+2) == cOrigin)) ||
                (((move->r-1) == rOrigin) && ((move->c+2) == cOrigin))){ 
                
                pthread_mutex_lock(&mutex);
                {
                    total_closed_tours++;
                }
                pthread_mutex_unlock(&mutex);
                #ifndef QUIET
                if (pthread_self()==move->mainThreadID) {printf("MAIN");}
                else {printf("%d", currTID);}
                printf(": Found an closed knight's tour; incremented total_closed_tours\n");
                #endif
            }
            
            else{ // OPEN KNIGHT'S TOUR
                pthread_mutex_lock(&mutex);
                {
                    total_open_tours++;
                }
                pthread_mutex_unlock(&mutex);
                #ifndef QUIET
                if (pthread_self()==move->mainThreadID) {printf("MAIN");}
                else {printf("T%d", currTID);}
                printf(": Found an open knight's tour; incremented total_open_tours\n");
                #endif
            }
            
            pthread_mutex_lock(&mutex);
            {
                max_squares = (move->m*move->n); // Should have visited all squares => return m*n
            }
            pthread_mutex_unlock(&mutex);
        }

        for (int i=0; i<move->m; i++) {free(*(move->board+i));}
        free(move->board);
    }

    
    else if (pCCount==1){ // If only one possible move to make: make the move (using the same thread)
        #if DEBUG_MODE
        printf("T%d: only one possible move...\n", currTID);
        #endif
        move->rNew = *(*(possCoords+0)+0);
        move->cNew = *(*(possCoords+0)+1);

        moving(move); //############################################################################## Do anything with return value???
    }


    
    else{ // If multiple possible moves to make: multi-threading for-loop
        #ifndef QUIET
        if (pthread_self()==move->mainThreadID) {printf("MAIN");}
        else {printf("T%d", currTID);}
        printf(": %d possible moves after move #%d; creating %d child threads...\n",
            pCCount, number_of_visited_squares(move->board, move->m, move->n), pCCount);
        #endif

        // #if DEBUG_MODE
        // printf("ORIGINAL:\n");
        // print_move(move);
        // #endif

        int i, rc;

        pthread_t* TIDs = calloc(pCCount, sizeof(pthread_t)); // Array for storing (real) pthread IDs to reference later

        for (i=0 ; i<pCCount ; i++){
            #if DEBUG_MODE
            printf("T%d: In loop, threading out for (%d,%d)...\n", currTID, *(*(possCoords+i)+0), *(*(possCoords+i)+1));
            #endif

            // Create a copy of the move struct that can be manipulated w/out altering the original data in memory
            move_t newMove = move_copy(move);
            #if DEBUG_MODE
            printf("~~ Showing copy after function:\n");
            printf("~~ m = %d // n = %d\n", newMove.m, newMove.n);
            printf("~~ r = %d // c = %d\n", newMove.r, newMove.c);
            printf("~~ rNew = %d // cNew = %d\n", newMove.rNew, newMove.cNew);
            #endif
            // Update the COPY's rNew and cNew
            newMove.rNew = *(*(possCoords+i)+0);
            newMove.cNew = *(*(possCoords+i)+1);
            
            // Create the thread with the copy
            rc = pthread_create(TIDs+i, NULL, moving, &newMove);
            if (rc!=0) {fprintf(stderr, "pthread_create() failed (%d)\n", rc);}

            // #ifdef NO_PARALLEL // If no parallel: join thread immediately after
            int* retVal;

            pthread_join(*(TIDs+i), (void**)&retVal);
            
            #ifndef QUIET
            if (pthread_self()==move->mainThreadID) {printf("MAIN");}
            else {printf("T%d", currTID);}
            printf(": T%d joined\n", *retVal);
            #endif

            free(retVal);
            // #endif
        }

        // #ifndef NO_PARALLEL // If parallel: wait for all child threads to complete/terminate
        // int* retVals = calloc(pCCount, sizeof(int*));

        // for (i=0 ; i<pCCount ; i++){
        //     int* retVal;

        //     pthread_join(*(TIDs+i), (void**)&retVal);
            
        //     #ifndef QUIET
        //     if (pthread_self()==move->mainThreadID) {printf("MAIN");}
        //     else {printf("T%d", currTID);}
        //     printf(": T%d joined\n", *retVal);
        //     #endif

        //     *(retVals+i) = *retVal;
        //     free(retVal);
        // }

        // for (i=0 ; i<pCCount ; i++){printf("%d ", *(retVals+i));}
        // printf("\n");
        // #endif

        free(TIDs);
    }


    // Free all other LOCAL dynamic memory
    // for (int i=0 ; i<pCCount ; i++) {free(*(possCoords+pCCount));}
    // free(possCoords);
    

    // Return exit status (currTID) - terminate child thread, return ret (a pointer)
    int* ret = calloc(1, sizeof(int)); // Be sure to free outside of pthread call!
    *ret = currTID;
    return ret;
    // This return calls pthread_exit() ==> pthread_exit(ret)
}





void print_board(char** board, int m, int n){
    if (!board || m<=0 || n<=0) {fprintf(stderr, "Invalid board dimensions\n"); return;}

    int i, j;
    // Print column indices (header)
    printf("~~   ");  // Space for row indices
    for (i=0 ; i<n ; i++) {printf(" %2d ", i);}  // Column numbers
    printf("\n");
    // Print top border
    printf("~~   +"); // Align with row numbers
    for (i=0 ; i<n ; i++) {printf("---+");}
    printf("\n");
    // Print each row
    for (i=0 ; i<m ; i++) {
        printf("~~%2d |", i); // Row number
        for (j=0 ; j<n ; j++) {printf(" %c |", *(*(board+i)+j) ? *(*(board+i)+j) : ' ');} // Print cell content (space if null character)
        printf("\n");
        // Print row border
        printf("~~   +");
        for (j=0 ; j<n ; j++) {printf("---+");}
        printf("\n");
    }
}

void print_move(move_t* move){
    printf("~~ ========================================\n");
    print_board(move->board, move->m, move->n);
    printf("~~ ========================================\n");
    printf("m = %d // n = %d\n", move->m, move->n);
    printf("r = %d // c = %d\n", move->r, move->c);
    printf("rNew = %d // cNew = %d\n", move->rNew, move->cNew);
    printf("~~ ========================================\n");
}

int number_of_visited_squares(char** board, int m, int n){
    int ret = 0;
    for (int i=0; i<m ; i++){
        for (int j=0 ; j<n ; j++){
            if (*(*(board+i)+j)=='X') {ret++;}
        }
    }
    return ret;
}

move_t move_copy(move_t* orig){
    move_t copy = {0};  // Initialize all fields to zero/null

    if (!orig || orig->m<=0 || orig->n<=0) {fprintf(stderr, "Invalid original move\n"); return copy;}

    // Allocate memory, copy the board
    char** copyBoard = calloc(orig->m, sizeof(char*));
    if (!copyBoard) {perror("calloc() failed for row pointers"); return copy;}
    for(int i=0 ; i<orig->m ; i++){
        *(copyBoard+i) = calloc(orig->n, sizeof(char));
        if (!(copyBoard+i)) {
            perror("calloc() failed for column memory");
            for (int j=0 ; j<i ; j++) {free(*(copyBoard+j));} // Cleanup already-allocated rows
            free(copyBoard);
            return copy;
        }
        memcpy(*(copyBoard+i), *(orig->board+i), orig->n*sizeof(char));
    }

    copy.board = copyBoard;
    copy.m = orig->m; copy.n = orig->n;
    copy.r = orig->r; copy.c = orig->c;
    copy.rNew = orig->rNew; copy.cNew = orig->cNew;
    copy.threadNumber = orig->threadNumber;
    copy.threadID = orig->threadID;
    copy.mainThreadID = orig->mainThreadID;

    return copy;
}