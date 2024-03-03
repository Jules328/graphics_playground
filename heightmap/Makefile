MAKE     := /usr/bin/make
CC       := /usr/bin/gcc

CPPFLAGS := -Iinclude
CFLAGS   := -Wall -g -O0

.PHONY: all clean
all: main.exe

main.exe: src/main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

clean:
	rm -rf main.exe
