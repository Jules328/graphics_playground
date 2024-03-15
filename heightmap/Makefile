MAKE     := /usr/bin/make
CC       := /usr/bin/gcc

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -Wall -g -O0
LDLIBS   := -lGL -lglfw3 -lm -lcglm

.PHONY: all clean
all: gl1.exe gl2.exe

gl1.exe: src/main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -D_GL_VERSION_=1 $< -o $@ $(LDLIBS)

gl2.exe: src/main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -D_GL_VERSION_=2 $< -o $@ $(LDLIBS)

clean:
	rm -rf *.exe *.d
