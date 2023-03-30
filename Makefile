CC = gcc
CFLAGS = -Wall -pedantic -std=c99

all: inode

inode: inode.c
	$(CC) $(CFLAGS) inode.c -g
