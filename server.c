#define _POSIX_C_SOURCE 200112L

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Constants.
#define LISTEN_QUEUE_SIZE 20

// Function prototypes.
uint8_t get_protocol(const char *str);
char *get_root_path(char *path);
void debug_server_input(uint8_t protocol, char *port, char *path);
int socket_new(const uint8_t protocol, const char *port);
static void termination_handler(int signum);

// Signal status.
volatile sig_atomic_t is_listening = true;

// Functions.
int main(int argc, char *argv[]) {
    // Read arguments. Can assume well-formed provided arguments.
    if (argc != 4) {
        fprintf(stderr, "usage: ./server [4 | 6] [port number] [path to web root]\n");
        exit(EXIT_FAILURE);
    }
    uint8_t s_protocol = get_protocol(argv[1]);
    char *s_port = argv[2];
    char *s_root_path = get_root_path(argv[3]);
    // debug_server_input(s_protocol, s_port, s_root_path);

    // Initialise listening socket.
    int sockfd = socket_new(s_protocol, s_port);

    // Register signal handler.
    struct sigaction new_action;
    new_action.sa_handler = termination_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGINT, &new_action, NULL);
    sigaction(SIGTERM, &new_action, NULL);
    sigaction(SIGHUP, &new_action, NULL);

    // Accept connections
    int client_sockfd;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    while (is_listening) {
        client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_sockfd < 0) {
            // Could not accept: continue accepting other connections.
            perror("accept");
            continue;
        }
    }

    close(sockfd);
    return 0;
}

static void termination_handler(int signum) {
    is_listening = false;
    return;
}

int socket_new(const uint8_t protocol, const char *port) {
    // Provide hints for socket intialisation.
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = protocol == 6 ? AF_INET6 : AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // Create a socket for listening
    int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Attempt to reuse port.
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind address to socket.
    if (bind(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);

    // Start listening.
    if (listen(sockfd, LISTEN_QUEUE_SIZE) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

unsigned long strtoul_strict(const char *str) {
    // Converts string to unsigned long. Strict: exits if string cannot be fully represented as a long.
    const int base = 10;
    errno = 0;
    char *endptr;
    unsigned long val = strtoul(str, &endptr, base);
    if ((errno == ERANGE && (val == ULONG_MAX || val == 0)) || (errno != 0 && val == 0)) {
        perror("strtoul_strict");
        exit(EXIT_FAILURE);
    }
    if (endptr == str) {
        fprintf(stderr, "strtol_strict: no digits found.\n");
        exit(EXIT_FAILURE);
    }
    if (*endptr != '\0') {
        fprintf(stderr, "strtol_strict: str contains trailing spaces.\n");
        exit(EXIT_FAILURE);
    }
    return val;
}

uint8_t get_protocol(const char *str) {
    // Converts string to ip protocol. Strict: exits if not a supported IP protocol.
    unsigned long val = strtoul_strict(str);
    if (val != 4 && val != 6) {
        fprintf(stderr, "server: not a supported protocol [4 | 6].\n");
        exit(EXIT_FAILURE);
    }
    return (uint8_t)val;
}

char *get_root_path(char *path) {
    // Check for existence of file
    DIR *root_dir = opendir(path);
    if (root_dir == NULL) {
        fprintf(stderr,
                "server: root path does not exist. exiting early since no requests can be served at this root.\n");
        exit(EXIT_FAILURE);
    }
    closedir(root_dir);
    return path;
}

void debug_server_input(uint8_t protocol, char *port, char *path) {
    printf("%d, %s, %s, eol\n", protocol, port, path);
    return;
}
