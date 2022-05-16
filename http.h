#ifndef HTTP_H
#define HTTP_H

#include "response.h"

response_t *make_response(const char *path_root, const char *request_buffer);

#endif // !HTTP_H