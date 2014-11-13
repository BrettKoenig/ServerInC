CC = gcc
FILES = client.c
OUT_EXE = client

all: client server
client: PartC/client.c
	$(CC) -o client PartC/client.c

server: PartC/server.c
	$(CC) -o server PartC/server.c

clean:
	rm -f *.o core

rebuild: clean build