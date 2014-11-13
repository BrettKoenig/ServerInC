CC = gcc
FILES = client.c
OUT_EXE = client

all: client server
client: client.c
	$(CC) -o client client.c

server: server.c
	$(CC) -o server server.c

clean:
	rm -f *.o core

rebuild: clean build