CC=gcc
CFLAGS=-std=c99 -O2 -Wall -Werror=vla -pthread -DNDEBUG

server: server.c server_looper.h server_looper.c response.h response.c http.h http.c
	$(CC) $(CFLAGS) -g -o server server.c server_looper.h server_looper.c response.h response.c http.h http.c

clean:
	rm -f server *.o

format:
	clang-format -i *.c *.h
