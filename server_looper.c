#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "http.h"
#include "response.h"
#include "server_looper.h"

// Macro constants.
#define LISTEN_QUEUE_SIZE 20
#define RECV_TIMEOUT_SECS 10

// Signal status.
volatile sig_atomic_t is_listening = true;

// Thread arguments.
typedef struct client_args_t {
    int fd;
    const char *root_path;
} client_args_t;

// Function prototypes.
int socket_new(const uint8_t protocol, const char *port);
static void termination_handler(int signum);
client_args_t *client_args_create(int fd, const char *root_path);
void *client_thread(void *arg);
int send_header(response_t *res, int client_sockfd);
off_t send_fd_file(response_t *res, int client_sockfd);

int server_loop(uint8_t s_protocol, const char *s_port, const char *s_root_path) {
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

    // Initialise timeout instance to be used across all connections.
    const struct timeval timeout = {.tv_sec = RECV_TIMEOUT_SECS, .tv_usec = 0};

    // Define pthread attribute template to spawn pthreads detached.
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    // Accept connections
    int client_sockfd;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    while (is_listening) {
        client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_sockfd < 0) {
            // Could not accept: continue accepting other connections.
            if (is_listening) {
                perror("accept");
            }
            continue;
        }

        // Set a timeout on the client socket. Should never error out.
        if (setsockopt(client_sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror("setsockopt: client");
            close(client_sockfd);
            continue;
        }

        client_args_t *client_args = client_args_create(client_sockfd, s_root_path);
        if (!is_listening || client_args == NULL) {
            close(client_sockfd);
            free(client_args);
            continue;
        }

        pthread_t thread;
        if (pthread_create(&thread, &thread_attr, client_thread, (void *)client_args) < 0) {
            perror("pthread_create");
            close(client_sockfd);
            free(client_args);
        }
    }

    // No longer listening, cleanup
    pthread_attr_destroy(&thread_attr);
    close(sockfd);

    return 0;
}

void *client_thread(void *arg) {
    client_args_t *client_args = (client_args_t *)arg;
    int client_sockfd = client_args->fd;
    const char *s_root_path = client_args->root_path;
    free(client_args);

    // Store received data in a buffer.
    request_t req = {.buffer = {'\0'},
                     .slash_ptr = NULL,
                     .last_ptr = NULL,
                     .space_ptr = NULL,
                     .has_valid_method = false,
                     .has_valid_httpver = false};

    int count = 0, total = 0;
    enum request_stage_t stage = BAD;
    while ((count = recv(client_sockfd, &req.buffer[total], sizeof(req.buffer) - total - 1, 0)) > 0) {
        total += count;
        stage = process_partial_request(&req, total);
        if (stage != RECVING) {
            break;
        }
    }

    if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        // Received an error with the socket that was NOT because of a timeout - drop this client.
        perror("recv");
        close(client_sockfd);
        return NULL;
    }

    // assert(count == 0);
    // Make response - if bad request, return 400 response.
    response_t *res = stage == VALID ? make_response(s_root_path, &req) : response_create_400();
    if (res == NULL) {
        // Occurs only with malloc failure.
        perror("null response");
        close(client_sockfd);
        return NULL;
    }

    // Send header to client.
    int bytes_sent = send_header(res, client_sockfd);

    // Send content only if sending headers was successful.
    if (bytes_sent == res->header_size && res->body_size > 0) {
        // Need to switch on either sending out a byte array (e.g. 400 message) or a file.
        switch (res->status) {
        case HTTP_200:
            send_fd_file(res, client_sockfd);
            break;
        default:
            // Other statuses have Content-Length: 0 for now.
            break;
        }
    }

    // Finished sending: free response and close the connection.
    response_free(res);
    close(client_sockfd);
    return NULL;
}

int send_header(response_t *res, int client_sockfd) {
    int bytes_sent = 0, bytes_left = res->header_size, n;
    while (bytes_sent < res->header_size) {
        n = send(client_sockfd, res->header + bytes_sent, bytes_left, 0);
        if (n < 0) {
            perror("send: header error");
            break;
        }
        bytes_sent += n;
        bytes_left -= n;
    }
    return bytes_sent;
}

off_t send_fd_file(response_t *res, int client_sockfd) {
    off_t bytes_sent_offset = 0;
    int bytes_left = res->body_size, n;
    while (bytes_sent_offset < res->body_size) {
        n = sendfile(client_sockfd, res->body_fd, &bytes_sent_offset, bytes_left);
        if (n < 0) {
            perror("sendfile: 200 entity-body error");
            break;
        }
        bytes_sent_offset += n;
        bytes_left -= n;
    }
    return bytes_sent_offset;
}

client_args_t *client_args_create(int fd, const char *root_path) {
    client_args_t *args = malloc(sizeof(*args));
    if (args == NULL) {
        perror("client_args_create: malloc");
        return NULL;
    }
    args->fd = fd;
    args->root_path = root_path;
    return args;
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

    struct addrinfo *p;
    int sockfd = -1;
    bool successful = false;
    for (p = result; (!successful) && (p != NULL); p = p->ai_next) {
        if (p->ai_family != hints.ai_family) {
            perror("protocol ai_family mismatch");
            continue;
        }

        // Open socket file descriptor.
        sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (sockfd < 0) {
            perror("socket");
            continue;
        }

        // Attempt to reuse port.
        int enable = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
            perror("setsockopt");
            close(sockfd);
            continue;
        }

        // Bind address to socket.
        if (bind(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
            perror("bind");
            close(sockfd);
            continue;
        }

        // Begin listening.
        if (listen(sockfd, LISTEN_QUEUE_SIZE) < 0) {
            perror("listen");
            close(sockfd);
            continue;
        }

        // Successfully started listening: choose the first address.
        successful = true;
    }

    freeaddrinfo(result);
    if (!successful || sockfd < 0) {
        perror("could not listen on any port");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}