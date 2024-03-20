MAKE     := /usr/bin/make
CC       := /usr/bin/gcc

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -Wall -g -O0
LDLIBS   := -lGL -lglfw3 -lm -lcglm

EXE := gl1 gl2

.PHONY: all clean
all: $(EXE)

gl1: src/main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -D_GL_VERSION_=1 $< -o $@ $(LDLIBS)

gl2: src/main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -D_GL_VERSION_=2 $< -o $@ $(LDLIBS)

clean:
	rm -rf $(EXE) *.d
