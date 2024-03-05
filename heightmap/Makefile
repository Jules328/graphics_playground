MAKE     := /usr/bin/make
CC       := /usr/bin/gcc

CPPFLAGS := -Iinclude
CFLAGS   := -Wall -g -O0
LDLIBS   := -lGL -lglfw3 -lm -lcglm

.PHONY: all clean
all: main.exe

main.exe: src/main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -rf main.exe
