CC = gcc
CFLAGS = -Wall -O3

all:
	$(CC) $(CFLAGS) -o screenkey-guard screenkey-guard.c

.PHONY: all
