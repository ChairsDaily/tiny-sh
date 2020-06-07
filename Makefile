
MAKEFLAGS += --silent
LIBS=-ledit
CC=gcc
CFLAGS=-std=c99 -Wall
PROGRAM=src/tinysh.c

.PHONY: build clean test 

default: build
build:
	- mkdir bin
	- gcc $(CFLAGS) $(PROGRAM) $(LIBS) -o bin/tinysh

test:
	./bin/tinysh

clean:
	-rm -rf bin

