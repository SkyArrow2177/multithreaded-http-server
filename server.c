#define _POSIX_C_SOURCE 200112L
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LISTEN_QUEUE_SIZE 20

uint8_t get_protocol(const char *str);
in_port_t get_port(const char *str);
char *get_root_path(char *path);
void debug_server_input(uint8_t protocol, char *port, char *path);
int socket_new(const uint8_t protocol, const char *port);

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

    // Initialise listening socket
    int sockfd = socket_new(s_protocol, s_port);
    if (listen(sockfd, LISTEN_QUEUE_SIZE) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    close(sockfd);
    return 0;
}

int socket_new(const uint8_t protocol, const char *port) {
    // Provide hints for socket intialisation.
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = get_port(port) == 6 ? AF_INET6 : AF_INET;
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

in_port_t get_port(const char *str) {
    // Converts string to port number. Strict: exits if not a valid port number
    unsigned long val = strtoul_strict(str);
    if (val > UINT16_MAX) {
        fprintf(stderr, "server: port number too large [0..65535].\n");
        exit(EXIT_FAILURE);
    }
    return (in_port_t)val;
}

char *get_root_path(char *path) {
    // Check for existence of file
    DIR *root_dir = opendir(path);
    bool is_valid = root_dir != NULL;
    closedir(root_dir);
    if (!is_valid) {
        fprintf(stderr,
                "server: root path does not exist. exiting early since no requests can be served at this root.\n");
        exit(EXIT_FAILURE);
    }
    // Remove trailing slash.
    size_t path_len = strlen(path);
    if (path_len > 0 && path[path_len - 1] == '/') {
        path[path_len - 1] = '\0';
    }
    return path;
}

void debug_server_input(uint8_t protocol, char *port, char *path) {
    printf("%d, %s, %s, eol\n", protocol, port, path);
    return;
}
