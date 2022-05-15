#ifndef HTTP_H
#define HTTP_H

int get_request_uri(const char *request_buffer, char *uri_dest);
bool uri_has_escape(const char *uri, int uri_len);

#endif // !HTTP_H