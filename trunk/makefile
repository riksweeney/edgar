VERSION = 0.1
RELEASE = 1
DEV = 1
LOCALE_DIR = usr/share/locale/

CFLAGS    = -Wall -pedantic -Werror -DVERSION=$(VERSION) -DRELEASE=$(RELEASE) -DDEV=$(DEV) -DINSTALL_PATH=\"$(INSTALL_PATH)\" -DLOCALE_DIR=\"$(LOCALE_DIR)\"
LFLAGS    = `sdl-config --libs` -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lz
#LFLAGS    = `sdl-config --libs` -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lz -llibintl
MAIN_OBJS = draw.o main.o
EDIT_OBJS = draw_editor.o main_editor.o cursor.o
CORE_OBJS  = animation.o audio.o collisions.o entity.o font.o game.o graphics.o init.o input.o inventory.o
CORE_OBJS += lift.o map.o player.o resources.o weather.o sprites.o bat.o properties.o custom_actions.o
CORE_OBJS += item.o status.o enemies.o hud.o random.o decoration.o chicken_feed.o chicken.o
CORE_OBJS += key_items.o record.o geometry.o chicken_trap.o target.o spawner.o rock.o pressure_plate.o door.o
CORE_OBJS += weak_wall.o switch.o line_def.o boulder_boss.o trigger.o music.o coal_pile.o objective.o level_exit.o
CORE_OBJS += spider.o rock_pile.o grub.o grub_boss.o save_point.o shrub.o projectile.o load_save.o
CORE_OBJS += compress.o global_trigger.o fireball.o wasp.o small_boulder.o dialog.o script.o villager.o
CORE_OBJS += main_menu.o widget.o borgan.o menu.o options_menu.o npc.o gib.o heart_container.o action_point.o
CORE_OBJS += falling_platform.o spitting_plant.o red_grub.o stalactite.o bomb.o jumping_plant.o explosion.o bomb_pile.o
CORE_OBJS += jumping_slime.o
PROG      = edgar
ED_PROG   = mapeditor
CXX       = gcc

# top-level rule to create the program.
all: redo_deps makefile.dep $(PROG) $(ED_PROG)

redo_deps:
	rm makefile.dep

makefile.dep : src/*/*.c
	for i in src/*.c src/*/*.c; do gcc -MM "$${i}"; done > $@
	
include makefile.dep

# compiling other source files.
%.o:
	$(CXX) $(CFLAGS) -c -s $<

# linking the program.
$(PROG): $(MAIN_OBJS) $(CORE_OBJS)
	$(CXX) $(MAIN_OBJS) $(CORE_OBJS) -o $(PROG) $(LFLAGS)
	
# linking the program.
$(ED_PROG): $(EDIT_OBJS) $(CORE_OBJS)
	$(CXX) $(EDIT_OBJS) $(CORE_OBJS) -o $(ED_PROG) $(LFLAGS)

# cleaning everything that can be automatically recreated with "make".
clean:
	rm $(PROG) $(ED_PROG) *.o
