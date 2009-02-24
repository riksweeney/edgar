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
#include "hud.h"
#include "record.h"
#include "objective.h"

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
	int go, i, mapID;

	/* Start up SDL */

	init("The Legend of Edgar");

	/* Call the cleanup function when the program exits */

	atexit(cleanup);

	go = TRUE;

	/* Load the resources */

	for (i=1;i<argc;i++)
	{
		if (strcmpignorecase("-record", argv[i]) == 0)
		{
			setRecordData(argv[i + 1]);

			i++;
		}

		else if (strcmpignorecase("-playback", argv[i]) == 0)
		{
			setReplayData(argv[i + 1]);

			i++;
		}

		else if (strcmpignorecase("-bmpwrite", argv[i]) == 0)
		{
			setScreenshotDir(argv[i + 1]);

			i++;
		}

		else
		{
			mapID = i;
		}
	}

	if (game.gameType == RECORDING)
	{
		setMapFile(argv[mapID]);
	}

	if (game.gameType != REPLAYING)
	{
		loadMap(argv[mapID]);
	}

	loadRequiredResources();

	/* Initialise the game variables */

	initGame();

	addObjective("Collect 10 pieces of Coal", "Landslide Trigger");

	/* Loop indefinitely for messages */

	game.startTicks = SDL_GetTicks();

	while (go == TRUE)
	{
		getInput(game.gameType);

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

		/* Do the HUD */

		doHud();

		/* Draw the map */

		draw();

		/* Sleep briefly to stop sucking up all the CPU time */

		delay(frameLimit);

		frameLimit = SDL_GetTicks() + 16;

		game.frames++;
	}

	/* Exit the program */

	exit(0);
}
