MAKE     := /usr/bin/make
CC       := /usr/bin/gcc

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -Wall -g -O0
LDFLAGS  := 
LDLIBS   := -lGL -lglfw -lm

SDIR := src
ODIR := obj

EXE  := graphics
SRC  := $(wildcard $(SDIR)/*.c)
OBJ  := $(SRC:$(SDIR)/%.c=$(ODIR)/%.o)

.PHONY: all clean
all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(ODIR)/%.o: $(SDIR)/%.c | $(ODIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(ODIR):
	mkdir -p $@

clean:
	rm -rf $(EXE) $(ODIR)
