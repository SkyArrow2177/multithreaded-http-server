CC=gcc
CFLAGS=-std=c99 -O2 -Wall -Werror=vla -lpthread

server: server.c response.h response.c http.h http.c
	$(CC) $(CFLAGS) -o server server.c response.h response.c http.h http.c

clean:
	rm -f server http_test *.o

format:
	clang-format -i *.c *.h

http_test: http_test.c response.h response.c http.h http.c
	$(CC) $(CFLAGS) -o http_test http_test.c response.h response.c http.h http.c