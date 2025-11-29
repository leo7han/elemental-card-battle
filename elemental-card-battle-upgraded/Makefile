
CC=gcc
CFLAGS=-Wall -pthread -g

all: server client

server:
	$(CC) server/server.c server/game.c server/state.c -o server.bin $(CFLAGS)

client:
	$(CC) client/client.c client/ui.c -o client.bin $(CFLAGS)

clean:
	rm -f *.bin
