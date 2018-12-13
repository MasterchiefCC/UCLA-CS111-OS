# NAME: CHEN CHEN
# EMAIL: chenchenstudent@gmail.com
# ID: 004710308
#-Wall -Wextra
CC = gcc
CFLAGS = -g3 -lm -lmraa

default:install

build:
	$(CC) lab4c_tcp.c -c $(CFLAGS)
	$(CC) lab4c_tls.c -c $(CFLAGS) -lssl -lcrypto

install:build
	$(CC) lab4c_tcp.o -o lab4c_tcp $(CFLAGS)
	$(CC) lab4c_tls.o -o lab4c_tls $(CFLAGS) -lssl -lcrypto

clean:
	rm -f lab4c-004710308.tar.gz lab4c_tcp lab4c_tls *.o

dist:
	tar -czvf lab4c-004710308.tar.gz lab4c_tls.c lab4c_tcp.c lab4c.h README Makefile 