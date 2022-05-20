#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "server_looper.h"

// Features:
#define IMPLEMENTS_IPV6
#define MULTITHREADED

// Function prototypes.
uint8_t get_protocol(const char *str);
char *get_root_path(char *path);
void debug_server_input(uint8_t protocol, char *port, char *path);

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

    // Run the server.
    server_loop(s_protocol, s_port, s_root_path);

    // Any detached threads will also be terminated when main returns:
    // https://man7.org/linux/man-pages/man3/pthread_detach.3.html (not code, just manpage)
    return 0;
}

unsigned long strtoul_strict(const char *str) {
    // Converts string to unsigned long with error checking.
    // Strict: exits if string cannot be fully represented as a long.
    const int base = 10;
    errno = 0;
    char *endptr;
    unsigned long val = strtoul(str, &endptr, base);

    // Error scenarios are described at https://man7.org/linux/man-pages/man3/strtoul.3.html (not code, just manpage).
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
    // uint8_t: a more compact representation, although an enum is also a decent solution.
    unsigned long val = strtoul_strict(str);
    if (val != 4 && val != 6) {
        fprintf(stderr, "server: not a supported protocol [4 | 6].\n");
        exit(EXIT_FAILURE);
    }
    return (uint8_t)val;
}

char *get_root_path(char *path) {
    // Check for existence of directory. Supports relative paths prefixed witih './'.
    // Does not copy the null-terminated string.
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
