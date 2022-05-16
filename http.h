#ifndef HTTP_H
#define HTTP_H

#include "stdbool.h"

#include "response.h"

#define REQUEST_SIZE 2400

enum request_stage_t { RECVING, VALID, BAD };
typedef struct request_t {
    char buffer[REQUEST_SIZE + 1];
    char *slash_ptr;
    char *last_ptr;
    char *space_ptr;
    bool has_valid_method;
} request_t;

enum request_stage_t process_partial_request(request_t *req);
response_t *make_response(const char *path_root, const char *request_buffer);

int get_request_uri(const char *request_buffer, char **uri_dest);
bool uri_has_escape(const char *uri, int uri_len);
const char *get_mime(const char *uri);
int get_path(const char *path_root, const char *uri, const int uri_len, char **path_dest);
int get_body_fd(const char *path);

#endif // !HTTP_H