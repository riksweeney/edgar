/*
Copyright (C) 2009 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "headers.h"

#include "init.h"
#include "game.h"
#include "system/resources.h"
#include "map.h"
#include "player.h"
#include "inventory.h"
#include "decoration.h"
#include "collisions.h"
#include "draw.h"
#include "input.h"
#include "entity.h"
#include "hud.h"
#include "system/record.h"
#include "event/objective.h"
#include "event/global_trigger.h"
#include "system/load_save.h"
#include "dialog.h"
#include "event/script.h"
#include "menu/menu.h"

Input input, menuInput;
Entity *self, entity[MAX_ENTITIES];
Animation animation[MAX_ANIMATIONS];
Game game;
Droplet droplet[MAX_DROPS];
Entity player, playerShield, playerWeapon;
Target target[MAX_TARGETS];
Control control;

int main(int argc, char *argv[])
{
	unsigned int frameLimit = SDL_GetTicks() + 16;
	int go, i, mapID, loadSave;

	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");
	textdomain("edgar");
	bindtextdomain("edgar", LOCALE_DIR);

	/* Start up SDL */

	init(_("The Legend of Edgar"));

	/* Call the cleanup function when the program exits */

	atexit(cleanup);

	go = TRUE;

	loadSave = FALSE;

	mapID = -1;

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

		else if (strcmpignorecase("-load", argv[i]) == 0)
		{
			loadSave = TRUE;

			i++;
		}
		#if DEV == 1
			else
			{
				mapID = i;
			}
		#endif
	}

	loadRequiredResources();

	if (loadSave == FALSE)
	{
		if (game.gameType == RECORDING)
		{
			setMapFile(mapID == -1 ? "map01" : argv[mapID]);
		}

		if (game.gameType != REPLAYING)
		{
			loadMap(mapID == -1 ? "map01" : argv[mapID], TRUE);
		}
	}

	else
	{
		if (loadGame(0) == FALSE)
		{
			printf("No saved game in slot 0\n");

			exit(1);
		}
	}

	/* Initialise the game variables */

	initGame();

	/* Loop indefinitely for messages */

	game.startTicks = SDL_GetTicks();

	#if DEV == 1
		printf("DEV Version\n");
	#else
		printf("Production Version\n");
	#endif

	while (go == TRUE)
	{
		getInput(game.gameType);

		switch (game.status)
		{
			case IN_GAME:
				freeCollisionGrid();

				doGame();

				doPlayer();

				doInventory();

				doMap();

				doEntities();

				doDecorations();

				doCollisions();

				doHud();
			break;

			case IN_MENU:
				doMenu();
			break;

			default:
				doMenu();
			break;
		}

		draw();

		/* Sleep briefly to stop sucking up all the CPU time */

		delay(frameLimit);

		frameLimit = SDL_GetTicks() + 16;

		game.frames++;
	}

	/* Exit the program */

	exit(0);
}
