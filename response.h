#ifndef RESPONSE_H
#define RESPONSE_H

#include <sys/types.h>

typedef struct response_t {
    enum response_status_t { HTTP_400, HTTP_404, HTTP_200 } status;
    char *header;
    char *body_buffer;
    int body_fd;
    size_t header_size;
    size_t body_size;
} response_t;

response_t *response_create_404();

response_t *response_create_200(int fd, const char *mime);

response_t *response_create_400();

void response_free(response_t *res);

#endif
