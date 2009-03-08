VERSION = 0.1
RELEASE = 1
DEV = 1
INSTALL_PATH = ""

CFLAGS    = -Wall -pedantic -Werror -DVERSION=$(VERSION) -DRELEASE=$(RELEASE) -DDEV=$(DEV) -DINSTALL_PATH=$(INSTALL_PATH)
LFLAGS    = `sdl-config --libs` -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lz
OBJS      = animation.o audio.o collisions.o draw.o entity.o font.o game.o graphics.o init.o input.o inventory.o
OBJS     += lift.o main.o map.o player.o resources.o weather.o sprites.o bat.o properties.o custom_actions.o
OBJS     += item.o status.o enemies.o hud.o random.o decoration.o chicken_feed.o chicken.o
OBJS     += key_items.o record.o geometry.o chicken_trap.o target.o spawner.o rock.o pressure_plate.o door.o
OBJS     += weak_wall.o switch.o line_def.o boulder_boss.o trigger.o music.o coal_pile.o objective.o level_exit.o
OBJS     += spider.o rock_pile.o grub.o grub_boss.o save_point.o shrub.o projectile.o load_save.o
OBJS     += compress.o global_trigger.o fireball.o wasp.o small_boulder.o
ED_OBJS   = animation.o audio.o collisions.o draw_editor.o entity.o font.o game.o graphics.o init.o input.o inventory.o
ED_OBJS  += lift.o main_editor.o map.o player.o resources.o weather.o sprites.o bat.o properties.o custom_actions.o
ED_OBJS  += item.o status.o cursor.o enemies.o hud.o random.o decoration.o chicken_feed.o chicken.o
ED_OBJS  += key_items.o record.o geometry.o chicken_trap.o target.o spawner.o rock.o pressure_plate.o door.o
ED_OBJS  += weak_wall.o switch.o line_def.o boulder_boss.o trigger.o music.o coal_pile.o objective.o level_exit.o
ED_OBJS  += spider.o rock_pile.o grub.o grub_boss.o save_point.o shrub.o projectile.o load_save.o
ED_OBJS  += compress.o global_trigger.o fireball.o wasp.o small_boulder.o
PROG      = edgar
ED_PROG   = mapeditor
CXX       = gcc

# top-level rule to create the program.
all: redo_deps makefile.dep $(PROG) $(ED_PROG)

redo_deps:
	rm makefile.dep

makefile.dep : src/*.c
	for i in src/*.c; do gcc -MM "$${i}"; done > $@
	
include makefile.dep

# compiling other source files.
%.o:
	$(CXX) $(CFLAGS) -c -s $<

# linking the program.
$(PROG): $(OBJS)
	$(CXX) $(OBJS) -o $(PROG) $(LFLAGS)
	
# linking the program.
$(ED_PROG): $(ED_OBJS)
	$(CXX) $(ED_OBJS) -o $(ED_PROG) $(LFLAGS)

# cleaning everything that can be automatically recreated with "make".
clean:
	rm $(PROG) $(ED_PROG) *.o
