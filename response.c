#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "response.h"

#define HTTP_VERSION "HTTP/1.0"
#define CRLF "\r\n"
#define HTTP_CLENGTH_PREFIX "Content-Length:"
#define HTTP_CTYPE_PREFIX "Content-Type:"
#define SP " "

char HTTP_404_HEADER[] = HTTP_VERSION SP "404 Not Found" CRLF HTTP_CLENGTH_PREFIX SP "0" CRLF CRLF;
char HTTP_400_HEADER[] = HTTP_VERSION SP "400 Bad Request" CRLF HTTP_CLENGTH_PREFIX SP "0" CRLF CRLF;
const char HTTP_200_HEADER[] =
    HTTP_VERSION SP "200 OK" CRLF HTTP_CLENGTH_PREFIX SP "%zu" CRLF HTTP_CTYPE_PREFIX SP "%s" CRLF CRLF;

static response_t *response_create() {
    // Intialisation for response.
    response_t *res = malloc(sizeof(*res));
    if (res == NULL) {
        return NULL;
    }

    res->header = NULL;
    res->header_size = 0;
    res->body_fd = -1;
    res->body_buffer = NULL;
    res->body_size = 0;

    return res;
}

response_t *response_create_404() {
    response_t *res = response_create();
    if (res == NULL) {
        return NULL;
    }

    // Save header only.
    res->status = HTTP_404;
    res->header = HTTP_404_HEADER;
    res->header_size = strlen(res->header);
    return res;
}

response_t *response_create_400() {
    response_t *res = response_create();
    if (res == NULL) {
        return NULL;
    }

    // Save header only.
    res->status = HTTP_400;
    res->header = HTTP_400_HEADER;
    res->header_size = strlen(res->header);
    return res;
}

response_t *response_create_200(int fd, const char *mime) {
    response_t *res = response_create();
    if (res == NULL) {
        close(fd);
        return NULL;
    }

    // Save status and fd for sendfile later.
    res->status = HTTP_200;
    res->body_fd = fd;

    // Get and store Entity-Body file size.
    struct stat st;
    fstat(fd, &st);
    res->body_size = st.st_size;

    // Format header in buffer of sufficient size.
    int size_needed = snprintf(NULL, 0, HTTP_200_HEADER, st.st_size, mime);
    if (size_needed < 0) {
        perror("snprintf: response_create_200");
        free(res);
        return NULL;
    }

    char *header = malloc(sizeof(*header) * (size_needed + 1));
    if (header == NULL) {
        // Malloc failure
        perror("malloc: response_create_200");
        free(res);
        return NULL;
    }
    snprintf(header, size_needed + 1, HTTP_200_HEADER, st.st_size, mime);

    // Update header in response
    res->header = header;
    res->header_size = size_needed;

    return res;
}

void response_free(response_t *res) {
    if (res == NULL) {
        return;
    }

    switch (res->status) {
    case HTTP_200:
        free(res->header);
        close(res->body_fd);
        break;
    default:
        break;
    }

    free(res);
    return;
}
