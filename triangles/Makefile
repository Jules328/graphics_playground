MAKE     := /usr/bin/make
CC       := /usr/bin/gcc

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -Wall -g -O0
LDFLAGS  := 
LDLIBS   := -lGL -lglfw -lm

SDIR := src
ODIR := obj

EXE  := triangle.exe triangle_sc1.exe triangle_sc2.exe

.PHONY: all clean
all: $(EXE)

triangle.exe: $(ODIR)/triangle.o $(ODIR)/glad.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.exe: $(ODIR)/%.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(ODIR)/%.o: $(SDIR)/%.c | $(ODIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(ODIR):
	mkdir -p $@

clean:
	rm -rf $(EXE) $(ODIR)
