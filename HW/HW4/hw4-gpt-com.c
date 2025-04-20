#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <stdatomic.h>

/* External global variables defined in hw4-main.c */
extern int game_token;
extern int total_wins;
extern int total_losses;
extern char** words;

/* Structure to track state for each active game */
struct game_state {
    int token;                   // Unique game identifier
    char* hidden_word;           // The secret word to guess
    int remaining_guesses;       // Countdown from 6 to 0
    struct sockaddr_in client_addr;  // Client's network address
};

/* Global game management */
static struct game_state* active_games = NULL;  // Dynamic array of active games
static size_t active_games_count = 0;           // Number of active games
static volatile sig_atomic_t shutdown_flag = 0; // Atomic flag for graceful shutdown

/* Signal handler for SIGUSR1 (trigger server shutdown) */
void handle_sigusr1(int sig) {
    shutdown_flag = 1;
}

/* Main server logic */
int wordle_server(int argc, char **argv) {
    /* [Argument validation omitted for brevity] */

    /* Load words from file into dictionary */
    char **dictionary = calloc(num_words, sizeof(char *));
    /* ... (code to validate and load words) ... */

    /* UDP socket setup */
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr = {/* configuration */};
    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    /* Signal configuration: Ignore SIGINT/SIGTERM, handle SIGUSR1 */
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    struct sigaction sa = {.sa_handler = handle_sigusr1};
    sigaction(SIGUSR1, &sa, NULL);

    printf("Wordle UDP server started\n");

    /* Main server loop */
    while (!shutdown_flag) {
        struct sockaddr_in client_addr;
        char buffer[1024];
        
        /* Receive UDP datagram */
        ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, 
                            (struct sockaddr *)&client_addr, &client_len);

        /* Handle "NEW" game request */
        if (n == 3 && memcmp(buffer, "NEW", 3) == 0) {
            game_token++;  // Global counter for unique game IDs
            
            /* Select random word from dictionary */
            int word_index = rand() % num_words;
            char *selected_word = dictionary[word_index];

            /* Expand global words array and store new word */
            words = realloc(words, (words_count + 2) * sizeof(char *));
            words[words_count] = calloc(6, sizeof(char));
            strncpy(words[words_count], selected_word, 5);

            /* Create new game state */
            struct game_state new_game = {
                .token = game_token,
                .hidden_word = words[words_count],  // Pointer to global storage
                .remaining_guesses = 6,
                .client_addr = client_addr  // Store client's address
            };

            /* Add to active games array */
            active_games = realloc(active_games, (active_games_count + 1) * sizeof(struct game_state));
            active_games[active_games_count++] = new_game;

            /* Send game token to client (network byte order) */
            int net_token = htonl(game_token);
            sendto(sockfd, &net_token, sizeof(net_token), 0, 
                  (struct sockaddr *)&client_addr, client_len);
        }

        /* Handle game guess (9-byte datagram) */
        else if (n == 9) {
            /* Extract token and guess from payload */
            int token = ntohl(*(int *)buffer);
            char guess[6];
            memcpy(guess, buffer + 4, 5);
            guess[5] = '\0';

            /* Find matching game state */
            struct game_state *game = NULL;
            for (size_t i = 0; i < active_games_count; i++) {
                if (active_games[i].token == token) {
                    game = &active_games[i];
                    break;
                }
            }

            /* Validate guess against dictionary */
            int valid_guess = 0;
            for (int i = 0; i < num_words; i++) {
                if (strcasecmp(dictionary[i], guess) == 0) {
                    valid_guess = 1;
                    break;
                }
            }

            /* Process valid guess */
            if (valid_guess) {
                game->remaining_guesses--;
                
                /* Calculate letter matches */
                char result[5] = "-----";
                int hidden_counts[26] = {0};
                
                // First pass: mark correct positions and count letters
                for (int i = 0; i < 5; i++) {
                    if (toupper(guess[i]) == game->hidden_word[i]) {
                        result[i] = toupper(guess[i]);
                    } else {
                        hidden_counts[game->hidden_word[i] - 'a']++;
                    }
                }

                // Second pass: mark misplaced letters
                for (int i = 0; i < 5; i++) {
                    if (result[i] != '-') continue;
                    char c = tolower(guess[i]);
                    if (hidden_counts[c - 'a'] > 0) {
                        result[i] = c;
                        hidden_counts[c - 'a']--;
                    }
                }

                /* Check for win condition */
                int win = strcasecmp(result, game->hidden_word) == 0;
                
                /* Prepare response packet */
                char response[12];
                *(int *)response = htonl(token);
                response[4] = 'Y';  // Valid guess
                *(short *)(response + 5) = htons(game->remaining_guesses);
                memcpy(response + 7, result, 5);

                /* Update game statistics */
                if (win || game->remaining_guesses == 0) {
                    (win ? total_wins : total_losses)++;
                    /* Remove game from active list */
                }
            }
        }
    }

    /* Cleanup on shutdown */
    close(sockfd);
    for (size_t i = 0; i < active_games_count; i++) {
        printf("Game %d terminated (word: %s)\n", 
              active_games[i].token, active_games[i].hidden_word);
    }
    /* Free all allocated memory */
    return EXIT_SUCCESS;
}