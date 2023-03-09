CC=gcc
CFLAGS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 -D_POSIX_C_SOURCE=200112L -std=c99 -O2 -Wall -Werror=vla -pthread -DNDEBUG -g

OBJ_SERVER = server_looper.o response.o http.o

server: server.c $(OBJ_SERVER)
	$(CC) $(CFLAGS) -o server $(OBJ_SERVER) $<

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f server *.o

format:
	clang-format -i *.c *.h
