CFLAGS    = -Wall -pedantic -Werror
LFLAGS    = `sdl-config --libs` -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf
OBJS      = animation.o audio.o collisions.o draw.o entity.o font.o game.o graphics.o init.o input.o inventory.o
OBJS     += lift.o main.o map.o player.o resources.o weapons.o weather.o sprites.o bat.o properties.o custom_actions.o
PROG      = edgar
CXX       = gcc

# top-level rule to create the program.
all: $(PROG) $(MAP_PROG)

# compiling other source files.
%.o: src/%.c src/%.h src/defs.h src/structs.h
	$(CXX) $(CFLAGS) -c -s $<

# linking the program.
$(PROG): $(OBJS)
	$(CXX) $(OBJS) -o $(PROG) $(LFLAGS)

# cleaning everything that can be automatically recreated with "make".
clean:
	rm $(PROG) $(MAP_PROG) *.o
