CC=gcc
CFLAGS= -g -pedantic -Wall 


all: main

run: main
	./main

main: main.o percent.o process.o get.o server.o
	$(CC) $(CFLAGS) -o main main.o get.o percent.o process.o server.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c 

get.o: get.c 
	$(CC) $(CFLAGS) -c get.c

percent.o: percent.c percent.h
	$(CC) $(CFLAGS) -c percent.c 

process.o: process.c 
	$(CC) $(CFLAGS) -c process.c

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

clean: 
	-/bin/rm -f main.o get.o percent.o process.o server.o main
