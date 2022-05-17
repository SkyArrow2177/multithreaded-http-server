#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "http.h"
#include "response.h"

#define BAD_REQUEST -1
#define NOT_FOUND_REQUEST -2
#define MIME_MAP_LEN 4
#define REQ_PREFIX "GET /"
#define REQ_PREFIX_LEN 5
#define REQ_HTTP10 " HTTP/1.0\r\n"
#define REQ_HTTP11 " HTTP/1.1\r\n"
#define REQ_HTTP_LEN 11

// Pre-computed mime map for simplicity and readibility.
const char *mime_map[MIME_MAP_LEN][2] = {
    {".html", "text/html"}, {".jpg", "image/jpeg"}, {".css", "text/css"}, {".js", "text/javascript"}};
const char mime_default[] = "application/octet-stream";

enum request_stage_t process_partial_request(request_t *req) {
    if (!req->has_valid_method) {
        // Need to first process the GET method with the starting / in abs_path.
        size_t buffer_len = strlen(req->buffer);
        if (buffer_len >= REQ_PREFIX_LEN) {
            // REQ_PREFIX must be a prefix of the buffer.
            if (strncmp(req->buffer, REQ_PREFIX, REQ_PREFIX_LEN) == 0) {
                // Proceed to checking checking for HTTP-Version
                req->has_valid_method = true;
                req->slash_ptr = req->buffer + REQ_PREFIX_LEN - 1;
                req->last_ptr = req->buffer + REQ_PREFIX_LEN;
            } else {
                return BAD;
            }

        } else {
            // The buffer is too small - but it must be a prefix of REQ_PREFIX.
            if (strncmp(req->buffer, REQ_PREFIX, buffer_len) == 0) {
                return RECVING;
            } else {
                return BAD;
            }
        }
    }

    // assert(req->has_valid_method);
    // The buffer is updated, so look for the first space
    // starting from where we began filling the buffer this recv call.
    if (req->space_ptr == NULL) {
        req->space_ptr = strchr(req->last_ptr, ' ');
    }

    if (req->space_ptr == NULL) {
        // Still couldn't find a space, update next recv's starting position.
        req->last_ptr += strlen(req->last_ptr);
        return RECVING;
    }

    size_t from_space_len = strlen(req->space_ptr);
    if (from_space_len >= REQ_HTTP_LEN) {
        // Must have entire HTTP-Version already in the buffer.
        if (strncmp(req->space_ptr, REQ_HTTP10, REQ_HTTP_LEN) == 0 ||
            strncmp(req->space_ptr, REQ_HTTP11, REQ_HTTP_LEN) == 0) {
            return VALID;

        } else {
            return BAD;
        }

    } else {
        // Check for partial prefix of HTTP-Version
        if (strncmp(req->space_ptr, REQ_HTTP10, from_space_len) == 0 ||
            strncmp(req->space_ptr, REQ_HTTP11, from_space_len) == 0) {
            return RECVING;

        } else {
            return BAD;
        }
    }
}

response_t *make_response(const char *path_root, const char *request_buffer) {
    if (path_root == NULL || request_buffer == NULL) {
        return NULL;
    }

    // Get URI from a well-formed request-line.
    // Allow for misformed headers to continue past this as long as the request-line is valid. Ed #887.
    char *uri = NULL;
    int uri_len = get_request_uri(request_buffer, &uri);
    if (uri_len == BAD_REQUEST) {
        return response_create_400();
    }

    // 404 URIs which traverse upwards the directory tree.
    if (uri_has_escape(uri, uri_len)) {
        return response_create_404();
    }

    // get full path.
    char *body_path = NULL;
    int path_len = get_path(path_root, uri, uri_len, &body_path);
    if (path_len < 0) {
        // Prefer giving 404 over crashing or existing on malloc failure.
        return response_create_404();
    }

    // attempt to open the file.
    int body_fd = get_body_fd(body_path);
    free(body_path);
    body_path = NULL;
    if (body_fd < 0) {
        free(uri);
        uri = NULL;
        return response_create_404();
    }

    // get mime type.
    const char *mime = get_mime(uri);
    free(uri);
    uri = NULL;

    // craft response.
    response_t *res_ok = response_create_200(body_fd, mime);

    return res_ok;
}

int get_request_uri(const char *request_buffer, char **uri_dest) {
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
    *uri_dest = uri;
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

const char *get_mime(const char *uri) {
    // Guaranteed that uri contains at least one '/' since previous checks for abs_path have been done.
    char *last_slash = strrchr(uri, '/');
    char *last_dot = strrchr(uri, '.');

    if (last_dot == NULL || last_slash > last_dot) {
        return mime_default;
    }

    // Linear search is very fast due to high locality and lookup-table compiler optimizations.
    for (int i = 0; i < MIME_MAP_LEN; i++) {
        if (strcmp(last_dot, mime_map[i][0]) == 0) {
            return mime_map[i][1];
        }
    }
    return mime_default;
}

int get_path(const char *path_root, const char *uri, const int uri_len, char **path_dest) {
    // Allocate full path
    int path_len = strlen(path_root) + uri_len;
    char *full_path = malloc(sizeof(*full_path) * (path_len + 1));
    if (full_path == NULL) {
        // This reeeeally shouldn't happen, but drop the request if necessary
        perror("malloc: get_path");
        return NOT_FOUND_REQUEST;
    }
    // Concat string. TODO: check if \0 is needed at the end.
    strcpy(full_path, path_root);
    strcat(full_path, uri);
    *path_dest = full_path;
    return path_len;
}

int get_body_fd(const char *path) {
    // Get file descriptor, if path points to a present filesystem location.
    int body_fd = open(path, O_RDONLY);
    if (body_fd < 0) {
        return NOT_FOUND_REQUEST;
    }

    // Check if regular file (as opposed to directory or FIFO).
    struct stat st;
    fstat(body_fd, &st);
    if (S_ISREG(st.st_mode)) {
        return body_fd;
    }
    // Not a regular file: clean up file descriptor and prepare 404.
    close(body_fd);
    return NOT_FOUND_REQUEST;
}