CC=gcc
CFLAGS=-lgmp -lpthread -O3

all: yprime

yprime: yprime.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) yprime
