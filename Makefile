CC=gcc
CFLAGS=-O2 -march=native
LDFLAGS=-lm -lalleg

SOURCE=$(wildcard *.c)
DEPENDENCY=$(wildcard *.h) Makefile

all: angry_moth

debug: CFLAGS += -DDEBUG -g
debug: angry_moth

angry_moth: $(SOURCE) $(DEPENDENCY)
	$(CC) $(SOURCE) $(CFLAGS) $(LDFLAGS) -o angry_moth
