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

uint8_t strtoi_protocol(const char *str);
in_port_t strtoi_port(const char *str);
char *get_root_path(char *path);
void debug_server_input(uint8_t protocol, in_port_t port, char *path);

int main(int argc, char *argv[]) {
    // Store arguments. Can assume well-formed provided arguments.
    if (argc != 4) {
        fprintf(stderr, "usage: ./server [4 | 6] [port number] [path to web root]\n");
        exit(EXIT_FAILURE);
    }
    uint8_t s_protocol = strtoi_protocol(argv[1]);
    in_port_t s_port = strtoi_port(argv[2]);
    char *s_path = get_root_path(argv[3]);

    debug_server_input(s_protocol, s_port, s_path);

    return 0;
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

uint8_t strtoi_protocol(const char *str) {
    // Converts string to ip protocol. Strict: exits if not a supported IP protocol.
    unsigned long val = strtoul_strict(str);
    if (val != 4 && val != 6) {
        fprintf(stderr, "server: not a supported protocol [4 | 6].\n");
        exit(EXIT_FAILURE);
    }
    return (uint8_t)val;
}

in_port_t strtoi_port(const char *str) {
    // Converts string to port number. Strict: exits if not a valid port number
    unsigned long val = strtoul_strict(str);
    if (val > UINT16_MAX) {
        fprintf(stderr, "server: port number too large [0..65535].\n");
        exit(EXIT_FAILURE);
    }
    return (in_port_t)val;
}

char *get_root_path(char *path) {
    DIR *root_dir = opendir(path);
    bool is_valid = root_dir != NULL;
    closedir(root_dir);
    if (!is_valid) {
        fprintf(stderr,
                "server: root path does not exist. exiting early since no requests can be served at this root.\n");
        exit(EXIT_FAILURE);
    }
    return path;
}

void debug_server_input(uint8_t protocol, in_port_t port, char *path) {
    printf("%d, %d, %s", protocol, port, path);
    return;
}
