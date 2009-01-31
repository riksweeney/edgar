#include "headers.h"

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

Input input;
Entity *self, entity[MAX_ENTITIES];
Animation animation[MAX_ANIMATIONS];
Game game;
Droplet droplet[MAX_DROPS];
Entity player, playerShield, playerWeapon;
Target target[MAX_TARGETS];

int main(int argc, char *argv[])
{
	unsigned int frameLimit = SDL_GetTicks() + 16;
	int go;

	/* Start up SDL */

	init("The Legend of Edgar");

	/* Call the cleanup function when the program exits */

	atexit(cleanup);

	go = 1;

	/* Load the resources */

	if (argc == 2)
	{
		loadMap(argv[1]);
	}

	else
	{
		loadMap("data/maps/map01.dat");
	}

	loadRequiredResources();

	/* Initialise the game variables */

	initGame();

	/* Loop indefinitely for messages */

	while (go == 1)
	{
		getInput();

		/* Do the game */

		doGame();

		/* Do the player, provided they still have enough lives left */

		doPlayer();

		/* Do the inventory */

		doInventory();

		/* Do the map */

		doMap();

		/* Do the Entities */

		doEntities();

		/* Do decorations */

		doDecorations();

		/* Do collisions */

		doCollisions();

		/* Draw the map */

		draw();

		/* Sleep briefly to stop sucking up all the CPU time */

		delay(frameLimit);

		frameLimit = SDL_GetTicks() + 16;
	}

	/* Exit the program */

	exit(0);
}
