CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude
LDFLAGS = $(shell test -e /usr/lib/x86_64-linux-gnu/libreadline.so && echo -lreadline || echo -l:libreadline.so.8)
SRC = src/main.c src/shell.c src/execute.c
BIN = bin/myshell
all: $(BIN)
$(BIN): $(SRC) include/shell.h
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)
clean:
	rm -rf bin
.PHONY: all clean
