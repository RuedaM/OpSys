#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdatomic.h>
#include <sys/socket.h>
#include <errno.h>

extern int game_token;
extern int total_wins;
extern int total_losses;
extern char** words;

struct game_state {
    int token;
    char *hidden_word;
    int remaining_guesses;
    struct sockaddr_in client_addr;
};

static struct game_state* active_games = NULL;
static size_t active_games_count = 0;
static volatile sig_atomic_t shutdown_flag = 0;

void handle_sigusr1(int sig) {shutdown_flag = 1;}





int wordle_server(int argc, char** argv) {
    #if DEBUG_MODE
    printf("~~ Checking args: %s, %s, %s, %s\n", *(argv+1), *(argv+2), *(argv+3), *(argv+4));
    #endif

    if (argc!=5){
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: ./hw4.out <UDP-server-port> <word-file-path> <num-words> <seed>\n");
        return EXIT_FAILURE;
    }

    // Validate UDP port (1–65535)
    int port = atoi(*(argv+1));
    if (port<1 || port>65535) {fprintf(stderr, "ERROR: Invalid port\n"); return EXIT_FAILURE;}

    // Validate word file path
    FILE *word_file = fopen(*(argv+2), "r");
    if (!word_file) {fprintf(stderr, "ERROR: Cannot open word file\n"); return EXIT_FAILURE;}

    // Validate num-words (> 0)
    int num_words = atoi(*(argv+3));
    if (num_words<=0) {fprintf(stderr, "ERROR: num-words must be > 0\n"); fclose(word_file); return EXIT_FAILURE;}

    // Seed the random generator
    unsigned int seed = atoi(*(argv+4));
    srand(seed);

    /* Load words from file into dictionary */
    char **dictionary = calloc(num_words, sizeof(char *));
    if (!dictionary){perror("calloc failed"); fclose(word_file); return EXIT_FAILURE;}

    for (int i=0 ; i<num_words ; i++){
        char *line_buf = calloc(256, sizeof(char));
        if (!fgets(line_buf, 256, word_file)){
            fprintf(stderr, "ERROR: Word file has fewer than %d words\n", num_words);
            fclose(word_file);
            for (int j=0 ; j<i ; j++) {free(*(dictionary+j));}
            free(dictionary);
            return EXIT_FAILURE;
        }

        // *(line+strcspn(line, "\r\n")) = '\0';
        // for (char* p = line; *p ; p++) *p = tolower(*p);
        // if (strlen(line)!=5){
        //     fprintf(stderr, "ERROR: Word file contains a word with length !=
        //     5\n"); fclose(word_file); for (int j=0 ; j<i ; j++)
        //     {free(*(dictionary+j));} free(dictionary); return EXIT_FAILURE;
        // }

        *(dictionary+i) = calloc(6, sizeof(char));
        if (!*(dictionary+i)){
            perror("calloc failed");
            fclose(word_file);
            for (int j=0 ; j<i ; j++) {free(*(dictionary+j));}
            free(dictionary);
            return EXIT_FAILURE;
        }
        strncpy(*(dictionary+i), line_buf, 5); // store up to 5 chars
        free(line_buf);
    }
    fclose(word_file);

    // Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd<0){
        perror("socket failed");
        for (int i=0 ; i<num_words ; i++) {free(*(dictionary+i));}
        free(dictionary);
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to all interfaces

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr))<0) {
        perror("bind failed");
        close(sockfd);
        for (int i=0 ; i<num_words ; i++) {free(*(dictionary+i));}
        free(dictionary);
        return EXIT_FAILURE;
    }

    // Sinal configuration
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    //signal(SIGUSR1, handle_sigusr1);

    struct sigaction sa;
    sa.sa_handler = handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, NULL)==-1) {
        perror("sigaction");
        close(sockfd);
        for (int i=0 ; i<num_words ; i++) {free(*(dictionary+i));}
        free(dictionary);
        return EXIT_FAILURE;
    }

    printf("Opened %s; read %d valid words\n", *(argv+2), num_words);
    printf("Wordle UDP server started\n");

    // Print the actual integer port in host byte order
    // printf("SERVER ADDRESS PORT: %d\n", ntohs(server_addr.sin_port));



    // Main server loop: repeatedly receive and process messages
    while (!shutdown_flag) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        char* buffer = calloc(1024, sizeof(char));
        if (!buffer) {perror("calloc failed"); return EXIT_FAILURE;}

        // Receive a single datagram
        ssize_t n = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)&client_addr, &client_len);
        if (n<0) {
            if (shutdown_flag) {free(buffer); break;}
            if (errno==EINTR) {free(buffer); continue;} // Signal interrupted the system call
            perror("recvfrom failed");
            free(buffer);
            continue;
        }

        // Check message for "NEW" or a 9-byte guess
        if (n==3 && memcmp(buffer, "NEW", 3)==0) {
            // Handle "NEW" game request
            game_token++; // Global counter for unique game IDs

            // Select random word from dictionary
            int word_index = rand() % num_words;
            char* selected_word = *(dictionary+word_index);

            // Expand global words array and store new word
            int words_count = 0;
            while (*(words+words_count)) {words_count++;}
            
            char **new_words = realloc(words, (words_count + 2) * sizeof(char *));
            if (!new_words) {perror("realloc failed"); continue;}
            words = new_words;
            
            *(words+words_count) = calloc(6, sizeof(char));
            if (!*(words+words_count)) {perror("calloc failed"); continue;}
            strncpy(*(words+words_count), selected_word, 5);
            *(words+words_count+1) = NULL;

            // Create new game state
            struct game_state new_game;
            new_game.token = game_token;
            new_game.hidden_word = *(words+words_count);
            new_game.remaining_guesses = 6;
            new_game.client_addr = client_addr;

            // Add to active games array
            struct game_state *temp = realloc(active_games, (active_games_count+1)*sizeof(struct game_state));
            if (!temp) {perror("realloc failed"); continue;}
            active_games = temp;
            *(active_games+active_games_count) = new_game;
            active_games_count++;

            // Send game token to client (network byte order)
            int net_token = htonl(game_token);
            sendto(sockfd, &net_token, sizeof(net_token), 0, (struct sockaddr*)&client_addr, client_len);

            printf("New game request; assigned game token %d\n", game_token);

        }
        
        
        // Handle game guess
        else if (n==9){
            // First 4 bytes = token (network order)
            // Last 5 bytes = guess
            int token;
            memcpy(&token, buffer, 4);
            token = ntohl(token);

            char* guess = calloc(6, sizeof(char));
            if (!guess) {perror("calloc failed"); return EXIT_FAILURE;}
            memcpy(guess, buffer+4, 5);
            // force to lower
            for (int j=0 ; j<5 ; j++) {*(guess+j) = tolower(*(guess+j));}

            // Locate the matching game state
            struct game_state *game = NULL;
            size_t i;
            
            for (i=0 ; i<active_games_count ; i++){
                if ((*(active_games+i)).token==token){
                    game = &(*(active_games+i));
                    break;
                }
            }

            printf("GAME %d: Received guess: %s\n", token, guess);

            // Build 12-byte response: (4-byte token) + (1-byte 'Y' or 'N')
            // + (2-byte #guesses) + (5-byte result)
            char *response = calloc(12, sizeof(char));
            if (!response) {perror("calloc failed"); exit(EXIT_FAILURE);}
            *(int*)response = htonl(token);

            if (!game) { // token not found
                *(response+4) = 'N';
                *(short*)(response+5) = htons(0); // 0 guesses left if not found
                memcpy(response + 7, "?????", 5);
                sendto(sockfd, response, 12, 0, (struct sockaddr *)&client_addr, client_len);
                continue;
            }

            // Validate the guess against the dictionary
            int valid_word = 0;
            for (int j=0 ; j<num_words ; j++){
                if (strncmp(*(dictionary+j), guess, 5)==0){
                    valid_word = 1;
                    break;
                }
            }

            if (!valid_word) { // Invalid guess
                *(response+4) = 'N';
                *(short*)(response + 5) = htons(game->remaining_guesses);
                memcpy(response + 7, "?????", 5);
                sendto(sockfd, response, 12, 0, (struct sockaddr *)&client_addr, client_len);
                
                printf("GAME %d: Invalid guess; sending reply: ????? (%d guesses left)\n", token, game->remaining_guesses);
                
                continue;
            }

            // Valid guess; decrement guess count
            game->remaining_guesses--;

            // Construct the result:
            // '-' = no match
            // lowercase = right letter, wrong placement
            // uppercase = fully correct
            char* result = calloc(6, sizeof(char));
            if (!result) {perror("calloc failed"); exit(EXIT_FAILURE);}
            for (int i=0 ; i<5 ; i++) {*(result+i) = '-';}
            *(result + 5) = '\0';

            int* hidden_counts = calloc(26, sizeof(int));
            for (int j=0 ; j<5 ; j++) {(*(hidden_counts + (*(game->hidden_word + j)-'a')))++;}

            // Mark correct letters
            for (int j=0 ; j<5 ; j++){
                if (*(guess+j)==(*(game->hidden_word + j))){
                    *(result+j) = toupper(*(guess+j));
                    (*(hidden_counts+(*(guess+j)-'a')))--;
                }
            }

            // Mark wrong-place letters
            for (int j=0 ; j<5 ; j++){
                if (*(result+j)=='-'){
                    int idx = *(guess+j) - 'a';
                    if (*(hidden_counts+idx)>0){
                        *(result+j) = tolower(*(guess+j));
                        (*(hidden_counts+idx))--;
                    }
                }
            }
            free(hidden_counts);

            // Check for win
            int correct = 1; // assume all correct
            for (int j=0 ; j<5 ; j++) {
                // if mismatch in uppercase letter
                if (*(result+j)!=toupper((*(game->hidden_word + j)))){
                    correct = 0;
                    break;
                }
            }

            // Build response packet
            *(response+4) = 'Y';
            *(short*)(response+5) = htons(game->remaining_guesses);
            memcpy(response + 7, result, 5);
            sendto(sockfd, response, 12, 0, (struct sockaddr *)&client_addr, client_len);

            printf("GAME %d: Sending reply: %s ", token, result);
            if (game->remaining_guesses==1) {printf("(%d guess left)\n", game->remaining_guesses);}
            else                            {printf("(%d guesses left)\n", game->remaining_guesses);}
            

            free(response);
            free(result);


            // If game over
            if (correct || (game->remaining_guesses==0)){
                if (correct) {total_wins++;}
                else {total_losses++;}

                printf("GAME %d: Game over; word was %s!\n", token, game->hidden_word);

                // Remove game from active_games
                if (i<active_games_count){
                    *(active_games+i) = *(active_games+active_games_count-1);
                    active_games_count--;
                    if (active_games_count==0){
                        free(active_games);
                        active_games = NULL;
                    }else{
                        struct game_state* temp_gs = realloc(active_games, active_games_count*sizeof(struct game_state));
                        // do not break if temp_gs is NULL, this might happen if active_games_count=0
                        if (temp_gs) {active_games = temp_gs;}
                    }
                }
            }

        } // end n==9 handling

        free(buffer);

    } // end while(!shutdown_flag)
    printf("SIGUSR1 received; Wordle server shutting down...\n");


    // Clean up after shutting down
    close(sockfd);

    // Any ongoing games become losses
    for (size_t i=0 ; i<active_games_count ; i++){
        printf("GAME %d: Game over; word was %s!\n", (*(active_games+i)).token, (*(active_games+i)).hidden_word);
        //total_losses++;
    }
    free(active_games);

    for (int i=0 ; i<num_words ; i++) {free(*(dictionary+i));}
    free(dictionary);

    return EXIT_SUCCESS;
}