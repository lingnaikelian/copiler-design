CC = gcc
CFLAGS = -Wall -g
BISON = bison
LDFLAGS = -lfl

TARGET = ../../bin/cc
SRCS = main.c symtab.c semantic.c ../parser/parser.tab.c ../parser/node.c ../parser/scanner.c ../scanner/scanner.c
OBJS = $(SRCS:.c=.o)

BISON_SRC = ../parser/parser.y

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p ../../bin
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

../parser/parser.tab.c ../parser/parser.tab.h: $(BISON_SRC)
	cd ../parser && $(BISON) -d parser.y

../scanner/scanner.c: ../scanner/scanner.l
	cd ../scanner && flex -o scanner.c scanner.l

../scanner/scanner.o: ../scanner/scanner.c ../scanner/common.h
	$(CC) $(CFLAGS) -c $< -o $@

../parser/node.o: ../parser/node.c ../parser/node.h ../parser/parser.tab.h
	$(CC) $(CFLAGS) -c $< -o $@

../parser/scanner.o: ../parser/scanner.c ../parser/node.h ../parser/parser.tab.h ../scanner/common.h
	$(CC) $(CFLAGS) -c $< -o $@

../parser/parser.tab.o: ../parser/parser.tab.c ../parser/node.h
	$(CC) $(CFLAGS) -c $< -o $@

symtab.o: symtab.c symtab.h

semantic.o: semantic.c semantic.h ../parser/node.h ../parser/parser.tab.h symtab.h

main.o: main.c ../parser/node.h ../scanner/common.h semantic.h ../parser/parser.tab.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean all

test: $(TARGET)
	./$(TARGET) test_s1.cmm

batch_test: $(TARGET)
	@echo "=== Running all 19 test cases ==="
	@for i in $$(seq 1 19); do \
		echo "--- test_s$${i}.cmm ---"; \
		./$(TARGET) test_s$${i}.cmm; \
		echo; \
	done

.PHONY: all clean rebuild test batch_test