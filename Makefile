CC=gcc
CFLAGS= -g -pedantic -Wall 


all: server

run: server
	./server start

server: server.o percent.o process.o get.o
	$(CC) $(CFLAGS) -o server server.o get.o percent.o process.o 

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

get.o: get.c 
	$(CC) $(CFLAGS) -c get.c

percent.o: percent.c percent.h
	$(CC) $(CFLAGS) -c percent.c 

process.o: process.c 
	$(CC) $(CFLAGS) -c process.c


utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c


clean: 
	-/bin/rm -f get.o percent.o process.o server.o utils.o server server.pid
