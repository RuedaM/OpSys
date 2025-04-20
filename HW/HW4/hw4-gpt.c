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
#include <signal.h>

#define _POSIX_C_SOURCE 199309L  // Required for sigaction

extern int game_token;
extern int total_wins;
extern int total_losses;
extern char** words;

struct game_state {
    int token;
    char* hidden_word;
    int remaining_guesses;
    struct sockaddr_in client_addr;
};

static struct game_state* active_games = NULL;
static size_t active_games_count = 0;
static volatile sig_atomic_t shutdown_flag = 0;

void handle_sigusr1(int sig) {
    shutdown_flag = 1;
}

int wordle_server(int argc, char **argv) {
#if DEBUG_MODE
    printf("~~ Checking args: %s, %s, %s, %s\n", *(argv+1),*(argv+2), *(argv+3), *(argv+4));
#endif

    if (argc != 5) {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: ./hw4.out <UDP-server-port> <word-file-path> <num-words> <seed>\n");
        return EXIT_FAILURE;
    }

    int port = atoi(*(argv + 1));
    if (port < 1 || port > 65535) {
        fprintf(stderr, "ERROR: Invalid port\n");
        return EXIT_FAILURE;
    }

    FILE *word_file = fopen(*(argv + 2), "r");
    if (!word_file) {
        fprintf(stderr, "ERROR: Cannot open word file\n");
        return EXIT_FAILURE;
    }

    int num_words = atoi(*(argv + 3));
    if (num_words <= 0) {
        fprintf(stderr, "ERROR: num-words must be > 0\n");
        fclose(word_file);
        return EXIT_FAILURE;
    }

    unsigned int seed = atoi(*(argv + 4));
    srand(seed);

    char **dictionary = calloc(num_words, sizeof(char *));
    if (!dictionary) {
        perror("calloc failed");
        fclose(word_file);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < num_words; i++) {
        char line[256];
        if (!fgets(line, sizeof(line), word_file)) {
            fprintf(stderr, "ERROR: Word file has fewer than %d words\n", num_words);
            fclose(word_file);
            for (int j = 0; j < i; j++) free(*(dictionary + j));
            free(dictionary);
            return EXIT_FAILURE;
        }
        line[strcspn(line, "\r\n")] = '\0';
        for (char *p = line; *p; p++) *p = tolower(*p);
        if (strlen(line) != 5) {
            fprintf(stderr, "ERROR: Word file contains a word with length !=5\n");
            fclose(word_file);
            for (int j = 0; j < i; j++) free(*(dictionary + j));
            free(dictionary);
            return EXIT_FAILURE;
        }
        *(dictionary + i) = calloc(6, sizeof(char));
        if (!*(dictionary + i)) {
            perror("calloc failed");
            fclose(word_file);
            for (int j = 0; j < i; j++) free(*(dictionary + j));
            free(dictionary);
            return EXIT_FAILURE;
        }
        strncpy(*(dictionary + i), line, 5);
    }
    fclose(word_file);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket failed");
        for (int i = 0; i < num_words; i++) free(*(dictionary + i));
        free(dictionary);
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        for (int i = 0; i < num_words; i++) free(*(dictionary + i));
        free(dictionary);
        return EXIT_FAILURE;
    }

    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);

    struct sigaction sa;  // Now compiles successfully
    sa.sa_handler = handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        close(sockfd);
        for (int i = 0; i < num_words; i++) free(*(dictionary + i));
        free(dictionary);
        return EXIT_FAILURE;
    }

    printf("Opened %s; read %d valid words\n", *(argv + 2), num_words);
    printf("Wordle UDP server started\n");

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[1024];

    while (!shutdown_flag) {
        ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_len);
        if (n == -1) {
            if (shutdown_flag) break;
            perror("recvfrom failed");
            continue;
        }

        if (n == 3 && memcmp(buffer, "NEW", 3) == 0) {
            game_token++;
            int new_token = game_token;

            int word_index = rand() % num_words;
            char *selected_word = *(dictionary + word_index);

            int words_count = 0;
            while (*(words + words_count)) words_count++;
            char **new_words = realloc(words, (words_count + 2) * sizeof(char *));
            if (!new_words) {
                perror("realloc failed");
                continue;
            }
            words = new_words;
            *(words + words_count) = calloc(6, sizeof(char));
            if (!*(words + words_count)) {
                perror("calloc failed");
                continue;
            }
            strncpy(*(words + words_count), selected_word, 5);
            *(words + words_count + 1) = NULL;

            struct game_state new_game = {
                .token = new_token,
                .hidden_word = *(words + words_count),
                .remaining_guesses = 6,
                .client_addr = client_addr
            };

            struct game_state *temp = realloc(active_games, (active_games_count + 1) * sizeof(struct game_state));
            if (!temp) {
                perror("realloc failed");
                continue;
            }
            active_games = temp;
            *(active_games + active_games_count) = new_game;
            active_games_count++;

            int net_token = htonl(new_token);
            sendto(sockfd, &net_token, sizeof(net_token), 0, (struct sockaddr *)&client_addr, client_len);
            printf("New game request; assigned game token %d\n", new_token);
        } else if (n == 9) {
            int token;
            memcpy(&token, buffer, 4);
            token = ntohl(token);
            char guess[6];
            memcpy(guess, buffer + 4, 5);
            guess[5] = '\0';
            for (char *p = guess; p < guess + 5; p++) *p = tolower(*p);

            struct game_state *game = NULL;
            size_t i;
            for (i = 0; i < active_games_count; i++) {
                if ((*(active_games + i)).token == token) {
                    game = active_games + i;
                    break;
                }
            }

            char response[12];
            *(int *)response = htonl(token);
            if (!game) {
                *(response + 4) = 'N';
                *(short *)(response + 5) = htons(0);
                memcpy(response + 7, "?????", 5);
                sendto(sockfd, response, 12, 0, (struct sockaddr *)&client_addr, client_len);
                continue;
            }

            int valid = 0;
            for (int j = 0; j < num_words; j++) {
                if (strncmp(*(dictionary + j), guess, 5) == 0) {
                    valid = 1;
                    break;
                }
            }

            if (!valid) {
                *(response + 4) = 'N';
                *(short *)(response + 5) = htons(game->remaining_guesses);
                memcpy(response + 7, "?????", 5);
                sendto(sockfd, response, 12, 0, (struct sockaddr *)&client_addr, client_len);
                printf("GAME %d: Received guess: %s\n", token, guess);
                printf("GAME %d: Invalid guess; sending reply: ????? (%d guesses left)\n", token, game->remaining_guesses);
                continue;
            }

            game->remaining_guesses--;

            char *result = calloc(5, sizeof(char));
            if (!result) {free(response); free(guess); perror("calloc failed"); continue;}
            memcpy(result, "-----", 5);
            int* hidden_counts = calloc(26, sizeof(int));
            char *hidden = game->hidden_word;

            for (int j = 0; j < 5; j++) {
                int idx = *(hidden + j) - 'a';
                *(hidden_counts + idx) += 1;
            }

            for (int j = 0; j < 5; j++) {
                if (*(guess + j) == *(hidden + j)) {
                    *(result + j) = toupper(*(guess + j));
                    int idx = *(guess + j) - 'a';
                    *(hidden_counts + idx) -= 1;
                }
            }

            for (int j = 0; j < 5; j++) {
                if (*(result + j) != '-') continue;
                char c = *(guess + j);
                int idx = c - 'a';
                if (*(hidden_counts + idx) > 0) {
                    *(result + j) = tolower(c);
                    *(hidden_counts + idx) -= 1;
                }
            }

            int correct = 1;
            for (int j = 0; j < 5; j++) {
                if (*(result + j) != toupper(*(hidden + j))) {
                    correct = 0;
                    break;
                }
            }

            *(response + 4) = 'Y';
            *(short *)(response + 5) = htons(game->remaining_guesses);
            memcpy(response + 7, result, 5);
            sendto(sockfd, response, 12, 0, (struct sockaddr *)&client_addr, client_len);

            printf("GAME %d: Received guess: %s\n", token, guess);
            printf("GAME %d: Sending reply: %s (%d guesses left)\n", token, result, game->remaining_guesses);

            if (correct || game->remaining_guesses == 0) {
                if (correct) {
                    total_wins++;
                } else {
                    total_losses++;
                }
                printf("GAME %d: Game over; word was %s!\n", token, game->hidden_word);
                if (i < active_games_count) {
                    *(active_games + i) = *(active_games + active_games_count - 1);
                    active_games_count--;
                    active_games = realloc(active_games, active_games_count * sizeof(struct game_state));
                }
            }
        }
    }

    close(sockfd);
    for (size_t i = 0; i < active_games_count; i++) {
        printf("GAME %d: Game over; word was %s!\n", (*(active_games + i)).token, (*(active_games + i)).hidden_word);
        total_losses++;
    }
    free(active_games);

    for (int i = 0; i < num_words; i++) free(*(dictionary + i));
    free(dictionary);

    return EXIT_SUCCESS;
}