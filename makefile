VERSION = 1.30
RELEASE = 1
DEV = 0
PAK_FILE = edgar.pak
DOCS = doc/*
ICONS = icons/
MAN = man/
UNIX = 1
MACOS = 0

ifeq ($(UNIX),0)
PROG      = edgar.exe
ED_PROG   = mapeditor.exe
PAK_PROG  = pak.exe
PO_PROG   = po_creator.exe
TILE_PROG = tile_creator.exe
else
PROG      = edgar
ED_PROG   = mapeditor
PAK_PROG  = pak
PO_PROG   = po_creator
TILE_PROG = tile_creator
endif

PREFIX = $(DESTDIR)/usr
BIN_DIR = $(PREFIX)/games/
DOC_DIR = $(PREFIX)/share/doc/$(PROG)/
ICON_DIR = $(PREFIX)/share/icons/hicolor/
DESKTOP_DIR = $(PREFIX)/share/applications/
APPDATA_DIR = $(PREFIX)/share/appdata/
MAN_DIR = $(PREFIX)/share/man/man6/
LOCALE_DIR = $(PREFIX)/share/locale/
LOCALE_MO = $(patsubst %.po,%.mo,$(wildcard locale/*.po))

ifeq ($(UNIX),0)
DATA_DIR =
else
DATA_DIR = $(PREFIX)/share/games/edgar/
endif

CFLAGS += -Wall -pedantic
ifeq ($(DEV),1)
CFLAGS += -Werror -g
endif

DEFINES = -DVERSION=$(VERSION) -DRELEASE=$(RELEASE) -DDEV=$(DEV) -DINSTALL_PATH=\"$(DATA_DIR)\" -DLOCALE_DIR=\"$(LOCALE_DIR)\" -DUNIX=$(UNIX)
ifndef NO_PAK
DEFINES += -DPAK_FILE=\"$(PAK_FILE)\"
endif

LDFLAGS += `sdl-config --libs` -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -lz -lm -lpng

TILE_OBJS  = tile_creator.o
PAK_OBJS   = pak_creator.o
PO_OBJS    = po_creator.o
MAIN_OBJS  = draw.o main.o
EDIT_OBJS  = draw_editor.o main_editor.o cursor.o
CORE_OBJS  = animation.o audio.o collisions.o entity.o font.o game.o graphics.o init.o input.o inventory.o
CORE_OBJS += lift.o map.o player.o resources.o weather.o sprites.o bat.o properties.o custom_actions.o
CORE_OBJS += item.o enemies.o hud.o random.o decoration.o chicken_feed.o chicken.o
CORE_OBJS += key_items.o record.o geometry.o chicken_trap.o target.o spawner.o rock.o pressure_plate.o door.o
CORE_OBJS += weak_wall.o switch.o line_def.o boulder_boss.o trigger.o music.o coal_pile.o objective.o level_exit.o
CORE_OBJS += spider.o grub.o grub_boss.o save_point.o shrub.o projectile.o load_save.o
CORE_OBJS += compress.o global_trigger.o fireball.o wasp.o small_boulder.o dialog.o script.o
CORE_OBJS += main_menu.o widget.o menu.o options_menu.o npc.o gib.o heart_container.o action_point.o
CORE_OBJS += falling_platform.o spitting_plant.o red_grub.o stalactite.o bomb.o explosion.o bomb_pile.o
CORE_OBJS += jumping_slime.o egg.o golem_boss.o baby_slime.o snail.o floating_snapper.o snake_boss.o
CORE_OBJS += enemy_generator.o flying_bug.o potions.o pak.o control_menu.o label.o sound_menu.o fly_boss.o ant_lion.o
CORE_OBJS += io_menu.o inventory_menu.o scorpion.o status_panel.o trap_door.o arrow_target.o eye_stalk.o snail_shell.o
CORE_OBJS += flying_maggot.o conveyor_belt.o power_generator.o laser_grid.o ceiling_snapper.o gazer.o
CORE_OBJS += teleporter.o sludge.o grabber.o golem_rock_dropper.o yes_no_menu.o about_menu.o summoner.o energy_drainer.o
CORE_OBJS += symbol_block.o number_block.o force_field.o gazer_eye_slot.o spring.o centurion.o
CORE_OBJS += ice_cube.o vanishing_platform.o armadillo.o bubble_machine.o bubble.o map_trigger.o potion_dispenser.o
CORE_OBJS += ceiling_crawler.o tesla_charger.o tesla_pack.o lightning_tortoise.o fire_burner.o high_striker.o containment_unit.o
CORE_OBJS += blob_boss.o containment_unit_controls.o ice_spray.o blending_machine.o robot.o anti_gravity_field.o spike_ball.o
CORE_OBJS += error.o code_door.o code_display.o skull.o poltergiest.o book.o moveable_lift.o glass_wall.o spike_wall.o
CORE_OBJS += whirlwind.o large_book.o tuning_fork.o pedestal.o spike_sphere.o cauldron.o soul_bottle.o exploding_spike_sphere.o
CORE_OBJS += thunder_cloud.o instruction_machine.o spirit.o black_book.o mataeus.o mataeus_wall.o mine_cart.o boulder_boss_2.o
CORE_OBJS += rock_container.o scale.o lava_geyzer.o large_spider.o crusher.o medal.o repellent.o cloud_geyzer.o
CORE_OBJS += ok_menu.o huge_spider.o armour_boss.o slime_container.o final_power_generator.o mirror.o light_source.o light_beam.o
CORE_OBJS += light_target.o balloon.o scanner.o fly_trap.o water_purifier.o extend_o_grab.o mouth_stalk.o ground_spear.o
CORE_OBJS += dragon_fly.o sliding_puzzle.o sliding_puzzle_display.o auto_spike_ball.o tread_mill.o power_meter.o gold_centurion.o
CORE_OBJS += shield_pillar.o crossbow.o crossbow_bolt.o catapult.o stats_menu.o awesome_boss.o awesome_boss_meter.o evil_edgar.o
CORE_OBJS += fish.o borer_boss.o light_pillar.o exploding_gazer_eye.o glass_cage.o lab_crusher.o exploding_gazer_eye_dud.o soul_merger.o
CORE_OBJS += soul_merger_control_panel.o fire_tortoise.o lava_door.o dark_summoner.o flame_pillar.o phoenix.o pendulum.o extend_o_grab_button.o
CORE_OBJS += puzzle_piece.o jigsaw_puzzle.o jigsaw_puzzle_display.o magnet.o ice_tortoise.o splitter.o attractor.o buzz_saw.o cell_bars.o
CORE_OBJS += sorceror.o centurion_boss.o sewer_boss.o item_remover.o safe.o ground_snapper.o sasquatch.o icicle.o giant_snowball.o
CORE_OBJS += script_menu.o snow_pile.o ceiling_creeper.o gas_plant.o centipede.o spore_crop.o spore.o sludge_pool.o vines.o undead_gazer.o
CORE_OBJS += slime_potion_pool.o frog.o master_tortoise.o red_ceiling_creeper.o rampaging_master_tortoise.o cheat_menu.o monster_skull.o
CORE_OBJS += cave_boss.o apple_tree.o apple.o one_way_door.o colour_pot.o colour_display.o phase_door.o mastermind_display.o mastermind.o
CORE_OBJS += iron_ball.o memory_match.o memory_match_display.o hoover.o black_book_2.o drawbridge_pulley.o drawbridge_anchor.o giant_ceiling_creeper.o
CORE_OBJS += baby_salamander.o safe_combination.o grimlore.o grimlore_artifact.o magic_missile.o grimlore_summon_spell.o hidden_passage_wall.o
CORE_OBJS += poison_meat.o train.o train_track.o flame_statue.o armour_changer.o credits.o black_book_3.o sorceror_2.o sorceror_dark_summoner.o
CORE_OBJS += sorceror_floor.o medals_menu.o skull_door.o ghost.o slug.o portable_save.o crypt_skull.o skeleton.o bell_rope.o
CORE_OBJS += music_sheet_display.o title.o azriel.o zombie.o mini_gargoyle.o gargoyle.o chaos.o chaos_vine.o i18n.o azriel_grave.o chaos_chain_base.o save_png.o

ifeq ($(UNIX),0)
CORE_OBJS += strtok_r.o
endif

# top-level rule to create the program.
.PHONY: all
all : $(PROG) makefile.dep $(ED_PROG) $(PAK_PROG) $(LOCALE_MO) $(PO_PROG)

makefile.dep : src/*/*.h src/*.h
	for i in src/*.c src/*/*.c; do $(CC) -MM "$${i}"; done > $@

# compiling other source files.
$(MAIN_OBJS) $(CORE_OBJS) $(EDIT_OBJS) $(TITLE_OBJS) $(PAK_OBJS) $(PO_OBJS) $(TILE_OBJS):
	$(CC) $(CFLAGS) $(DEFINES) -c -s $<

%.mo: %.po
	msgfmt -c -o $@ $<

# linking the program.
$(PROG): $(MAIN_OBJS) $(CORE_OBJS)
	$(CC) $(MAIN_OBJS) $(CORE_OBJS) -o $(PROG) $(LDFLAGS)

# linking the program.
$(ED_PROG): $(EDIT_OBJS) $(CORE_OBJS)
	$(CC) $(EDIT_OBJS) $(CORE_OBJS) -o $(ED_PROG) $(LDFLAGS)

# linking the program.
$(PAK_PROG): $(PAK_OBJS)
	$(CC) $(PAK_OBJS) -o $(PAK_PROG) $(LDFLAGS)

# linking the program.
$(PO_PROG): $(PO_OBJS)
	$(CC) $(PO_OBJS) -o $(PO_PROG) $(LDFLAGS)

# linking the program.
$(TILE_PROG): $(TILE_OBJS)
	$(CC) $(TILE_OBJS) -o $(TILE_PROG) $(LDFLAGS)

# cleaning everything that can be automatically recreated with "make".
clean:
	$(RM) $(PROG) $(ED_PROG) $(PAK_PROG) $(PO_PROG) $(TILE_PROG) $(PAK_FILE) $(LOCALE_MO) $(TILE_PROG) *.o makefile.dep

buildpak: $(PAK_PROG)
ifndef NO_PAK
	./$(PAK_PROG) data gfx music sound font $(PAK_FILE)
	./$(PAK_PROG) -test $(PAK_FILE)
endif

# install
install: all
ifeq ($(DEV),1)
	echo Cannot install if DEV is set to 1!
else
	$(MAKE) buildpak
	mkdir -p $(BIN_DIR)
	mkdir -p $(DATA_DIR)
	mkdir -p $(DOC_DIR)
	mkdir -p $(ICON_DIR)16x16/apps
	mkdir -p $(ICON_DIR)32x32/apps
	mkdir -p $(ICON_DIR)48x48/apps
	mkdir -p $(ICON_DIR)64x64/apps
	mkdir -p $(DESKTOP_DIR)
	mkdir -p $(APPDATA_DIR)
	mkdir -p $(MAN_DIR)

	cp $(PROG) $(BIN_DIR)$(PROG)
ifndef NO_PAK
	cp $(PAK_FILE) $(DATA_DIR)$(PAK_FILE)
else
	cp -a data gfx music sound font $(DATA_DIR)
endif
	cp $(DOCS) $(DOC_DIR)
	cp $(ICONS)16x16.png $(ICON_DIR)16x16/apps/$(PROG).png
	cp $(ICONS)32x32.png $(ICON_DIR)32x32/apps/$(PROG).png
	cp $(ICONS)48x48.png $(ICON_DIR)48x48/apps/$(PROG).png
	cp $(ICONS)64x64.png $(ICON_DIR)64x64/apps/$(PROG).png
	cp $(ICONS)$(PROG).desktop $(DESKTOP_DIR)
	cp $(ICONS)$(PROG).appdata.xml $(APPDATA_DIR)
	install -m 0644 $(MAN)$(PROG).6 $(MAN_DIR)

	@for f in $(LOCALE_MO); do \
		lang=`echo $$f | sed -e 's/^locale\///;s/\.mo$$//'`; \
		mkdir -p $(LOCALE_DIR)$$lang/LC_MESSAGES; \
		echo "cp $$f $(LOCALE_DIR)$$lang/LC_MESSAGES/$(PROG).mo"; \
		cp $$f $(LOCALE_DIR)$$lang/LC_MESSAGES/$(PROG).mo; \
	done
endif

uninstall:
	$(RM) $(BIN_DIR)$(PROG)
	$(RM) -r $(DATA_DIR)
	$(RM) -r $(DOC_DIR)
	$(RM) $(ICON_DIR)$(ICONS)$(PROG).png
	$(RM) $(ICON_DIR)16x16/apps/$(PROG).png
	$(RM) $(ICON_DIR)32x32/apps/$(PROG).png
	$(RM) $(ICON_DIR)48x48/apps/$(PROG).png
	$(RM) $(ICON_DIR)64x64/apps/$(PROG).png
	$(RM) $(DESKTOP_DIR)$(PROG).desktop
	$(RM) $(APPDATA_DIR)$(PROG).appdata.xml
	$(RM) $(MAN_DIR)/$(PROG).6*

	@for f in $(LOCALE_MO); do \
		lang=`echo $$f | sed -e 's/^locale\///;s/\.mo$$//'`; \
		echo "$(RM) $(LOCALE_DIR)$$lang/LC_MESSAGES/$(PROG).mo"; \
		$(RM) $(LOCALE_DIR)$$lang/LC_MESSAGES/$(PROG).mo; \
	done

-include makefile.dep
