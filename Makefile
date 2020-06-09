
MAKEFLAGS += --silent
LIBS=-ledit
CC=gcc
CFLAGS=-std=c99 -Wall
PROGRAM=src/tinysh.c

.PHONY: build clean test sync

default: build
build:
	- mkdir bin
	- gcc $(CFLAGS) $(PROGRAM) $(LIBS) -o bin/tinysh

test:
	./bin/tinysh

sync:
	- bash sync.sh "update"

clean:
	-rm -rf bin

