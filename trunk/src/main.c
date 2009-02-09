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
	int go, i;

	/* Start up SDL */

	init("The Legend of Edgar");

	/* Call the cleanup function when the program exits */

	atexit(cleanup);

	go = 1;

	/* Load the resources */
	
	for (i=1;i<argc;i++)
	{
		printf("%s\n", argv[i]);
		
		if (strcmpignorecase("-record", argv[i]) == 0)
		{
			setRecordData(argv[i + 1]);
			
			i++;
		}
		
		else if (strcmpignorecase("-replay", argv[i]) == 0)
		{
			setReplayData(argv[i + 1]);
			
			i++;
		}
		
		else
		{
			loadMap(argv[i]);
		}
	}

	loadRequiredResources();

	/* Initialise the game variables */

	initGame();
	
	/* Set replay data */

	/* Loop indefinitely for messages */

	while (go == 1)
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
	}

	/* Exit the program */

	exit(0);
}
