CC = gcc
CFLAGS = -Wall -g
BISON = bison
LDFLAGS = -lfl

TARGET = parser
SRCS = main.c node.c parser.tab.c scanner.c ../scanner/scanner.c
OBJS = $(SRCS:.c=.o)

BISON_SRC = parser.y

TEST_FILE = test.cmm

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

parser.tab.c parser.tab.h: $(BISON_SRC)
	$(BISON) -d $(BISON_SRC)

../scanner/scanner.c: ../scanner/scanner.l
	cd ../scanner && flex -o scanner.c scanner.l

../scanner/scanner.o: ../scanner/scanner.c ../scanner/common.h
	$(CC) $(CFLAGS) -c $< -o $@

node.o: node.c node.h parser.tab.h

scanner.o: scanner.c node.h parser.tab.h ../scanner/common.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) parser.tab.c parser.tab.h ../scanner/scanner.o

rebuild: clean all

test: $(TARGET)
	@if [ -f $(TEST_FILE) ]; then \
	    ./$(TARGET) $(TEST_FILE); \
	else \
	    echo "Test file $(TEST_FILE) not found. Please create it."; \
	    exit 1; \
	fi

.PHONY: all clean rebuild test
