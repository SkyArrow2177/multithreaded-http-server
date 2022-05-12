CC=gcc
CFLAGS=-std=c99 -Wall -O2 -lpthread

server: server.c
	$(CC) $(CFLAGS) -o server server.c

clean:
	rm -f server *.o

format:
	clang-format -i *.c *.h
