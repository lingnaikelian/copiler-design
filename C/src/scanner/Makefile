CC = gcc
FLEX = flex
CFLAGS = -Wall -g
LFLAGS = 

all: mkdir_bin scanner

mkdir_bin:
	mkdir -p ../../bin

scanner: scanner.o main.o
	$(CC) $(CFLAGS) -o ../../bin/scanner scanner.o main.o $(LFLAGS)

scanner.o: scanner.c common.h
	$(CC) $(CFLAGS) -c scanner.c

scanner.c: scanner.l
	$(FLEX) -o scanner.c scanner.l

main.o: main.c common.h
	$(CC) $(CFLAGS) -c main.c

test: scanner test.cmm
	../../bin/scanner test.cmm

clean:
	rm -f scanner.c scanner.o main.o

.PHONY: all clean test
