# NAME: CHEN CHEN
# EMAIL: chenchenstudent@gmail.com
# ID: 004710308
CC = gcc
CFLAGS = -g3 -lm -lmraa -Wall -Wextra

default:build

build:
	$(CC) lab4b.c -o lab4b $(CFLAGS)

clean: 
	rm -f lab4b *.tar.gz

dist: 
	tar -czvf lab4b-004710308.tar.gz lab4b.h lab4b.c Makefile README

tests:
	{ echo "START"; sleep 5; echo "SCALE=F"; sleep 5; echo "STOP"; sleep 5; echo "OFF"; } | ./lab4b --period=1 --log=log.txt --scale=C
	rm -f log.txt
