#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "response.h"

#define BAD_REQUEST -1

response_t *make_response(const char *path_root, const char *request_buffer) {
    char *uri = NULL;
    int uri_len = get_request_uri(request_buffer, uri);
    if (uri_len == BAD_REQUEST) {
        return response_create_400();
    }

    
}

int get_request_uri(const char *request_buffer, char *uri_dest) {
    // Check that the string starts with a GET method with an abs_path URI;
    char *line_start = strstr(request_buffer, "GET /");
    if (line_start != request_buffer) {
        return BAD_REQUEST;
    }

    // Check for CRLF in Request-Line.
    char *first_crlf = strstr(request_buffer, "\r\n");
    if (first_crlf == NULL) {
        return BAD_REQUEST;
    }

    // Start at the path, look for HTTP version.
    char *uri_start = line_start + 4;
    char *uri_space_http10 = strstr(uri_start, " HTTP/1.0\r\n");
    char *uri_space_http11 = strstr(uri_start, " HTTP/1.1\r\n");
    bool http10_bad = uri_space_http10 == NULL || uri_space_http10 >= first_crlf;
    bool http11_bad = uri_space_http11 == NULL || uri_space_http11 >= first_crlf;
    if (http10_bad && http11_bad) {
        return BAD_REQUEST;
    }

    // Copy uri path to new array.
    char *uri_space_http_any = http10_bad ? uri_space_http11 : uri_space_http10;
    ssize_t uri_len = uri_space_http_any - uri_start;
    char *uri = malloc(sizeof(*uri) * (uri_len + 1));
    if (uri == NULL) {
        perror("malloc: get_request_uri");
        return BAD_REQUEST;
    }
    strncpy(uri, uri_start, uri_len);
    uri[uri_len] = '\0';
    uri_dest = uri;
    return uri_len;
}