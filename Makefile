CC = gcc
CFLAGS = -Wall -g

all: demographics

demographics: main.o
	$(CC) $(CFLAGS) -o demographics main.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o demographics
