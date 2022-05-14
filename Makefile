CC=gcc
CFLAGS=-std=c99 -O2 -Wall -lpthread

server: server.c
	$(CC) $(CFLAGS) -o server server.c

clean:
	rm -f server *.o

format:
	clang-format -i *.c *.h
