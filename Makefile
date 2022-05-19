CC=gcc
CFLAGS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -D_POSIX_C_SOURCE=200112L -std=c99 -O2 -Wall -Werror=vla -pthread -DNDEBUG -g

server: server.c server_looper.h server_looper.c response.h response.c http.h http.c
	$(CC) $(CFLAGS) -o server server.c server_looper.h server_looper.c response.h response.c http.h http.c

clean:
	rm -f server *.o

format:
	clang-format -i *.c *.h
