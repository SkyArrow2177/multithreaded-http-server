#ifndef HTTP_H
#define HTTP_H

#include <stdbool.h>
#include <stddef.h>

#include "response.h"

#define REQUEST_SIZE 2400

// A HTTP Request parsing, processing, local file handling, and response construction library.

// A request object and state enum which encapsulates request data and partial-processing progress.
enum request_stage_t { RECVING, VALID, BAD };
typedef struct request_t {
    char buffer[REQUEST_SIZE + 1];
    char *slash_ptr;
    char *last_ptr;
    char *space_ptr;
    bool has_valid_method;
    bool has_valid_httpver;
} request_t;

// Process partial requests as they are updated on-the-fly, caching previous progress for improved performance.
enum request_stage_t process_partial_request(request_t *req, size_t buffer_len);

// Given a valid processes request object, extract and validate its URI for additional rules (path escape),
// open the file, get its mime, and build the response.
response_t *make_response(const char *path_root, const request_t *req);

#endif // !HTTP_H