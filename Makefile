CC=gcc
CFLAGS= -g -pedantic -Wall 


all: server

run: server
	./server start

server: server.o process.o get.o utils.o
	$(CC) $(CFLAGS) -o server server.o get.o process.o utils.o

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

get.o: get.c 
	$(CC) $(CFLAGS) -c get.c

process.o: process.c process.h
	$(CC) $(CFLAGS) -c process.c


utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c


clean: 
	-/bin/rm -f get.o process.o server.o utils.o server server.pid
