#ifndef SERVER_LOOPER_H
#define SERVER_LOOPER_H

#include <stdint.h>

// All network-related system calls are orchestrated in server_looper.c - this achieves good separation of
// concerns. We handoff request data processing work to functions in other modules as necessary. sendfile() benefits
// are described at the call site.

// The main loop of the HTTP server.
int server_loop(uint8_t s_protocol, const char *s_port, const char *s_root_path);

#endif // !SERVER_LOOPER_H
