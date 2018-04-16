SOURCES=map_generation.c render.c
DEBUG=-g
LIBS=-lSDL2 -lm
CFLAGS=-Wall -pedantic $(DEBUG)
OBJECTS=map_generation.o render.o
CC=gcc
EXE=lis

all: $(OBJECTS)
	@echo "Building the program..."
	$(CC) $(CFLAGS) -o $(EXE) $(OBJECTS) $(LIBS)

# This is the part that tells the make file to only compile when it's changed
map_generation.o: map_generation.c
	@echo "Compiling the map generation file..."
	$(CC) $(CFLAGS) -c map_generation.c

render.o: render.c
	@echo "Compiling the render file..."
	$(CC) $(CFLAGS) -c render.c

.PHONY: clean
clean:
	@echo "Cleaning up all the crap you have lying around in here..."
	rm -f *.o *.gch $(EXE)
