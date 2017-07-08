CC=gcc
CFLAGS= -g -pedantic -Wall 


all: webserver

run: webserver
	./webserver start

webserver: server.o process.o get.o utils.o
	$(CC) $(CFLAGS) -o webserver server.o get.o process.o utils.o

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

get.o: get.c 
	$(CC) $(CFLAGS) -c get.c

process.o: process.c process.h
	$(CC) $(CFLAGS) -c process.c


utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c


clean: 
	-/bin/rm -f get.o process.o server.o utils.o webserver server.pid
