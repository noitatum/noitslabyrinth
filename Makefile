.PHONY: all clean
CC = gcc
CFLAGS = -g -Wall -Werror -std=c11 -lSDL2
LDFLAGS =
SOURCES = $(wildcard *.c)
EXECUTABLES = $(SOURCES:.c=)

all: $(EXECUTABLES)

$(EXECUTABLES): $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $@.c -o $@

clean:
	rm -rf $(EXECUTABLES) $(OBJECTS)
