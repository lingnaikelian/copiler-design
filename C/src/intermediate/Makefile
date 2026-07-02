CC = gcc
CFLAGS = -Wall -g -I../parser -I../scanner -I../semantic
BISON = bison
LDFLAGS = -lfl

TARGET = ../../bin/cc
SRCS = main.c ir.c ../parser/node.c ../parser/parser.tab.c ../parser/scanner.c ../scanner/scanner.c ../semantic/semantic.c ../semantic/symtab.c
OBJS = main.o ir.o ../parser/node.o ../parser/parser.tab.o ../parser/scanner.o ../scanner/scanner.o ../semantic/semantic.o ../semantic/symtab.o

BISON_SRC = ../parser/parser.y

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p ../../bin
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

../parser/parser.tab.c ../parser/parser.tab.h: $(BISON_SRC)
	cd ../parser && $(BISON) -d $(BISON_SRC)

../scanner/scanner.c: ../scanner/scanner.l
	cd ../scanner && flex -o scanner.c scanner.l

../scanner/scanner.o: ../scanner/scanner.c ../scanner/common.h
	$(CC) $(CFLAGS) -c $< -o $@

../parser/node.o: ../parser/node.c ../parser/node.h ../parser/parser.tab.h
	$(CC) $(CFLAGS) -c $< -o $@

../parser/parser.tab.o: ../parser/parser.tab.c ../parser/parser.tab.h ../parser/node.h
	$(CC) $(CFLAGS) -c $< -o $@

../parser/scanner.o: ../parser/scanner.c ../parser/node.h ../parser/parser.tab.h ../scanner/common.h
	$(CC) $(CFLAGS) -c $< -o $@

../semantic/semantic.o: ../semantic/semantic.c ../semantic/semantic.h ../parser/node.h ../parser/parser.tab.h
	$(CC) $(CFLAGS) -c $< -o $@

../semantic/symtab.o: ../semantic/symtab.c ../semantic/symtab.h
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c ir.h ../parser/node.h ../parser/parser.tab.h

ir.o: ir.c ir.h ../parser/node.h ../parser/parser.tab.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) *.o ../parser/parser.tab.c ../parser/parser.tab.h ../scanner/scanner.o ../scanner/scanner.c ../semantic/semantic.o ../semantic/symtab.o

rebuild: clean all

test: $(TARGET)
	./$(TARGET) test.cmm

.PHONY: all clean rebuild test
