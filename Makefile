CC=gcc
CFLAGS=-std=c99 -O2 -Wall -Werror=vla -lpthread

server: server.c response.h response.c
	$(CC) $(CFLAGS) -o server server.c response.h response.c

clean:
	rm -f server *.o

format:
	clang-format -i *.c *.h
