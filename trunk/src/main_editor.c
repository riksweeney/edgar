#include "headers.h"

#include "cursor.h"
#include "init.h"
#include "game.h"
#include "resources.h"
#include "map.h"
#include "player.h"
#include "inventory.h"
#include "decoration.h"
#include "collisions.h"
#include "draw.h"
#include "input.h"
#include "entity.h"
#include "status.h"

Input input;
Entity player, playerWeapon, playerShield, *self, entity[MAX_ENTITIES];
Animation animation[MAX_ANIMATIONS];
Sound sound[MAX_SOUNDS];
Game game;
Inventory inventory;
Droplet droplet[MAX_DROPS];
Cursor cursor;
Target target[MAX_TARGETS];

int main(int argc, char *argv[])
{
	unsigned int frameLimit = SDL_GetTicks() + 16;
	int go;

	/* Start up SDL */

	init("Map Editor");

	/* Call the cleanup function when the program exits */

	atexit(cleanup);

	go = 1;

	if (argc == 2)
	{
		loadMap(argv[1]);
	}

	else
	{
		loadMap("data/maps/map01.dat");
	}

	/* Load the resources */

	loadRequiredResources();

	/* Load the background image */

	setMaxMapX(MAX_MAP_X * TILE_SIZE);
	setMaxMapY(MAX_MAP_Y * TILE_SIZE);

	/* Initialise the cursor */

	initCursor();

	/* Loop indefinitely for messages */

	while (go == 1)
	{
		/* Get the input */

		getInput();

		/* Do the cursor */

		doCursor();

		/* Do the status panel */

		doStatusPanel();

		/* Draw the map */

		draw();

		/* Sleep briefly to stop sucking up all the CPU time */

		delay(frameLimit);

		frameLimit = SDL_GetTicks() + 16;
	}

	/* Exit the program */

	exit(0);
}
