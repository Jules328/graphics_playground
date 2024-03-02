GL_LDFLAGS=-lGL -lglfw

graphics: src/main.c
	gcc -o $@ $^ $(GL_LDFLAGS)
