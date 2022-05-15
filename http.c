#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "response.h"

#define BAD_REQUEST -1

response_t *make_response(const char *path_root, const char *request_buffer) {
    // Get URI from a well-formed request-line.
    // Allow for misformed headers to continue past this as long as the request-line is valid. Ed #887.
    char *uri = NULL;
    int uri_len = get_request_uri(request_buffer, uri);
    if (uri_len == BAD_REQUEST) {
        return response_create_400();
    }

    // 404 URIs which traverse upwards the directory tree.
    if (uri_has_escape(uri, uri_len)) {
        return response_create_404();
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
    char *uri_space_next = strchr(uri_start, ' ');

    // We have a badly-formed Request-Line if we can't find HTTP-version,
    // or if it is not in the first line
    // or the next space does not also have HTTP after it.
    bool http10_bad = uri_space_http10 == NULL || uri_space_next != uri_space_http10 || uri_space_http10 >= first_crlf;
    bool http11_bad = uri_space_http11 == NULL || uri_space_next != uri_space_http11 || uri_space_http11 >= first_crlf;
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

bool strsuffix(const char *str, size_t str_len, const char *suffix, size_t suffix_len) {
    // Takes two non-empty strings and returns true if str's suffix equals `suffix`.
    if (str == NULL || suffix == NULL || str_len <= 0 || suffix_len <= 0) {
        return false;
    }
    if (suffix_len > str_len) {
        return false;
    }
    return strncmp(str + str_len - suffix_len, suffix, suffix_len) == 0;
}

bool uri_has_escape(const char *uri, int uri_len) {
    // True: contains escape, hence is a bad request. False: no escape, OK to continue handling.

    // Check for trailing "/.." escape.
    bool has_trailing_escape = strsuffix(uri, uri_len, "/..", 3);
    if (has_trailing_escape) {
        return true;
    }

    // Check for /../ (No need to check for leading ../ since abs_path must begin with /).
    char *ret = strstr(uri, "/../");
    bool has_middle_escape = ret != NULL;
    return has_middle_escape;
}