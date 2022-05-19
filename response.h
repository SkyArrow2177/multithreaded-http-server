#ifndef RESPONSE_H
#define RESPONSE_H

#include <sys/types.h>

// Response objects which encapsulate all the data necessary for the server to form a request to be directly written
// back to a client. This ensures separation of concerns by letting one module handle all system calls, and another
// module handle request string processing.

typedef struct response_t {
    enum response_status_t { HTTP_400, HTTP_404, HTTP_200 } status;
    char *header;
    char *body_buffer;
    int body_fd;
    size_t header_size;
    off_t body_size;
} response_t;

response_t *response_create_404();

response_t *response_create_200(int fd, const char *mime);

response_t *response_create_400();

void response_free(response_t *res);

#endif
