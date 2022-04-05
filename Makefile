CC = gcc
C_FLAGS = -Wall -g -O2 -Wextra -Wconversion -pedantic

SOURCES = $(shell find src/ -name *.c)

.PHONY: clean

build:
	$(CC) $(C_FLAGS) $(SOURCES) -o bin/blockchain
clean:
	rm -f bin/blockchain