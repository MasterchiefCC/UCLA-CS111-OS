# NAME: CHEN CHEN
# EMAIL: chenchenstudent@gmail.com
# ID: 004710308
CC = gcc
CFLAGS = -g3 -O0 -Wall -Wextra

default:build

build:
	$(CC) lab3a.c -o lab3a $(CFLAGS)

dist:
	tar -czvf lab3a-004710308.tar.gz lab3a.c ext2_fs.h README Makefile 

clean:
	rm -f lab3a lab3a-004710308.tar.gz