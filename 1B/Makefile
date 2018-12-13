CC = gcc
CFLAGS = -g3 -O0 -Wall -Wextra -lmcrypt

default: client server

client:
	$(CC) lab1b-client.c -o lab1b-client $(CFLAGS)

server:
	$(CC) lab1b-server.c -o lab1b-server $(CFLAGS)

clean:
	rm -f lab1b-server lab1b-client *.tar.gz *.o

dist:
	tar -zcvf lab1b-004710308.tar.gz lab1b-client.c lab1b-server.c my.key Makefile README