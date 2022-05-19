#ifndef SERVER_LOOPER_H
#define SERVER_LOOPER_H

#include <stdint.h>

int server_loop(uint8_t s_protocol, const char *s_port, const char *s_root_path);

#endif // !SERVER_LOOPER_H
