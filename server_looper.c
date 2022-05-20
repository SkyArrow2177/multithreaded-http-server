#include <assert.h>
#include <errno.h>
#include <limits.h>
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
void setup_signal_handling();

// The main loop of the HTTP server.
// All network-related system calls are orchestrated by functions in this file - this achieves good separation of
// concerns. We handoff request data processing work to functions in other modules as necessary. sendfile() benefits
// are described at the call site.
int server_loop(uint8_t s_protocol, const char *s_port, const char *s_root_path) {
    // Initialise listening socket.
    int sockfd = socket_new(s_protocol, s_port);

    // Register termination upon SIGINT and SIGTERM, and ignore SIGPIPE from clients.
    setup_signal_handling();

    // Initialise timeout instance to be used for receiving requests from each client.
    const struct timeval timeout = {.tv_sec = RECV_TIMEOUT_SECS, .tv_usec = 0};

    // Define pthread attribute template to spawn pthreads detached by default.
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

    // Accept connections from clients
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

        // Set a receive timeout on the client socket before handing off to thread. Should never error out.
        if (setsockopt(client_sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror("setsockopt: client");
            close(client_sockfd);
            continue;
        }

        // Prepare thread-local arguments for serving clients.
        client_args_t *client_args = client_args_create(client_sockfd, s_root_path);
        if (!is_listening || client_args == NULL) {
            close(client_sockfd);
            free(client_args);
            continue;
        }

        // Spawn a detached thread to receive and process a client request.
        pthread_t thread;
        if (pthread_create(&thread, &thread_attr, client_thread, (void *)client_args) < 0) {
            perror("pthread_create");
            close(client_sockfd);
            free(client_args);
        }
    }

    // No longer listening, clean-up the server.
    // pthread attributes are copied into each thread, so it is safe to free the thread attributes instance, even if a
    // thread runs before calling close(sockfd) on the server socket.
    // https://pubs.opengroup.org/onlinepubs/009695399/functions/pthread_create.html (not code, just manpage).
    pthread_attr_destroy(&thread_attr);
    close(sockfd);

    return 0;
}

// Thread function for receiving and processing client requests and orchestrating the delivery of responses.
void *client_thread(void *arg) {
    // Unwrap thread arguments.
    client_args_t *client_args = (client_args_t *)arg;
    int client_sockfd = client_args->fd;
    const char *s_root_path = client_args->root_path;
    free(client_args);

    // Store received data in a buffer allocated on the stack for better locality and performance.
    request_t req = {.buffer = {'\0'},
                     .slash_ptr = NULL,
                     .last_ptr = NULL,
                     .space_ptr = NULL,
                     .has_valid_method = false,
                     .has_valid_httpver = false};

    // Receive data from client on a loop, processing partial requests from multiple packets as soon as possible, and
    // storing this in a request instance.
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

    // count may not necessarily be zero here - e.g. received bytes successfully but stage = BAD due to early
    // elimination of a malformed request. Make response - if bad request, return 400 response.
    response_t *res = stage == VALID ? make_response(s_root_path, &req) : response_create_400();
    if (res == NULL) {
        // Occurs only with malloc failure - drop the client.
        perror("null response");
        close(client_sockfd);
        return NULL;
    }

    // Send header to client.
    int bytes_sent = send_header(res, client_sockfd);
    // Send content only if sending headers was successful.
    if (bytes_sent == res->header_size && res->body_size > 0) {
        // Switch on either sending out a byte array (e.g. 400 message with Entity-Body) or a file.
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

// Send the content of a header to a client. int types are fine since the headers in the server's response are bounded
// by the Linux absolute path limit of 4096 characters.
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

// Send binary file content to a client. On 64-bit systems and 32-bit systems with FILE_OFFSET_BITS=64 defined,
// off_t, sendfile, open are automatically converted to their 64-bit versions, enabling Large File Support.
off_t send_fd_file(response_t *res, int client_sockfd) {
    off_t bytes_sent_offset = 0L;
    off_t bytes_left = res->body_size;
    size_t count;
    ssize_t n;
    while (bytes_sent_offset < res->body_size) {
        bytes_left = res->body_size - bytes_sent_offset;
        // n's narrower type & the limit of SSIZE_MAX comes from sendfile sending at most SSIZE_MAX bytes per call. With
        // sendfile, there's no need to pass in an offset, but the max number of bytes to send should still be tracked.
        count = bytes_left > SSIZE_MAX ? SSIZE_MAX : bytes_left;

        // Why sendfile()?
        // sendfile() is more performant than the usual read() + send() loop. With read() + send(), we have to copy
        // kernel memory from the file resource into a per-thread userspace buffer, then back to kernel memory, and in
        // this process make two system calls (thus two context switches) per iteration. Smaller per-thread userspace
        // buffers significantly increase the number of iterations required. With sendfile(), data from the source file
        // descriptor is directly passed to the client socket entirely within the kernel space with an upper bound of
        // half the number of system calls, and lower memory usage since there's no need for a per-thread userspace
        // memory buffer. In practice, since the kernel cache memory buffer is much larger than a thread's
        // stack-allocated user-space buffer, we can send more data per sendfile() call compared to the read+send
        // solution, thus for a given file, the number of sendfile() calls is dramatically less than half the number of
        // read()/send() calls, thus improving performance further. In terms of application code simplicity, sendfile()
        // also transparently handles the correct file offset which is useful for handling large files (>2GB) where
        // multiple sendfile() calls are required. There is also no need to think about choosing a buffer that fits
        // within the stack or isn't too large for mallocing on the heap when there are many clients.

        // Some implementation notes: since sendfile can update the offset of the file descriptor, you should NOT pass
        // in an offset variable, but instead let the offset be NULL and let sendfile() update the fd's offset for you.
        // https://linux.die.net/man/2/sendfile (not code, just manpage). If you erroneously pass in a non-null offset,
        // you will notice that the returned bytes_send_offset will be double the correct value, but the actual offset
        // used by sendfile will grow exponentially, leading to failed downloads on files >2GB.
        n = sendfile(client_sockfd, res->body_fd, NULL, count);
        if (n <= 0) {
            perror("sendfile: 200 entity-body error");
            break;
        }
        bytes_sent_offset += n;
    }
    return bytes_sent_offset;
}

// Safely create the arguments for a thread.
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

// Signal handler function for termination, which flips a flag.
static void termination_handler(int signum) {
    is_listening = false;
    return;
}

// Register signal handlers.
void setup_signal_handling() {
    // Register signal handler for termination.
    struct sigaction new_action;
    new_action.sa_handler = termination_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGINT, &new_action, NULL);
    sigaction(SIGTERM, &new_action, NULL);
    sigaction(SIGHUP, &new_action, NULL);

    // Ignore SIGPIPE (e.g. from a curl client sending Ctrl-C).
    struct sigaction ignored_signals;
    ignored_signals.sa_handler = SIG_IGN;
    sigemptyset(&ignored_signals.sa_mask);
    ignored_signals.sa_flags = 0;
    sigaction(SIGPIPE, &ignored_signals, NULL);
}

// Establishes server socket for listening.
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

    // Bind and set socket to listen.
    struct addrinfo *p;
    int sockfd = -1;
    bool successful = false;
    // Loop through to ensure that an address with the correct protocol version is used.
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

        // Attempt to reuse port. Required per Project 2 Specification.
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