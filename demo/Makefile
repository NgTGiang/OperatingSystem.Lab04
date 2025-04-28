all: main

CFLAGS = -pthread -fcommon
CC = gcc $(CFLAGS)

main: fjpool fjtask fjworker
	$(CC) -c -o main.o main.c
	$(CC) -o fjpool main.o fjpool.o fjtask.o fjworker.o

fjpool: fjpool.c fjpool.h
	$(CC) -c -o fjpool.o fjpool.c

fjtask: fjtask.c fjpool.h
	$(CC) -c -o fjtask.o fjtask.c

fjworker: fjworker.c fjpool.h
    $(CC) -c -o fjworker.o fjworker.c

clean:
	rm *.o
	rm mypool
