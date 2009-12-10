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
#include "graphics/decoration.h"
#include "collisions.h"
#include "draw.h"
#include "input.h"
#include "entity.h"
#include "hud.h"
#include "system/record.h"
#include "event/objective.h"
#include "event/global_trigger.h"
#include "system/load_save.h"
#include "system/error.h"
#include "dialog.h"
#include "event/script.h"
#include "menu/menu.h"
#include "menu/inventory_menu.h"
#include "system/error.h"

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
	unsigned int frameLimit;
	char *firstMap;
	int go, i, mapID, loadSlot, recordingID, replayingID;

	#ifndef NO_GETTEXT
		printf("Locale is %s\n", setlocale(LC_ALL, ""));
		printf("Numeric is %s\n", setlocale(LC_NUMERIC, "C"));
		printf("atof(2.75) is %f\n", atof("2.75"));
		textdomain("edgar");
		bindtextdomain("edgar", LOCALE_DIR);
	#endif

	/* Start up SDL */

	init(_("The Legend of Edgar"));

	/* Call the cleanup function when the program exits */

	atexit(cleanup);

	go = TRUE;

	loadSlot = -1;
	
	game.fps = 1000 / 60;

	mapID = recordingID = replayingID = -1;

	/* Load the resources */

	for (i=1;i<argc;i++)
	{
		if (strcmpignorecase("-record", argv[i]) == 0)
		{
			if (recordingID == -1)
			{
				recordingID = i + 1;
			}

			i++;
		}

		else if (strcmpignorecase("-playback", argv[i]) == 0)
		{
			if (replayingID == -1)
			{
				replayingID = i + 1;
			}

			i++;
		}

		else if (strcmpignorecase("-bmpwrite", argv[i]) == 0)
		{
			setScreenshotDir(argv[i + 1]);

			i++;
		}

		else if (strcmpignorecase("-load", argv[i]) == 0)
		{
			loadSlot = atoi(argv[i + 1]);

			i++;
		}
		#if DEV == 1
			else
			{
				mapID = i;
			}
		#endif
	}

	if (replayingID != -1 && recordingID != -1)
	{
		showErrorAndExit("Cannot record and replay at the same time");
	}

	loadRequiredResources();

	if (loadSlot == -1)
	{
		firstMap = game.firstRun == TRUE ? "tutorial" : "map01";

		if (recordingID != -1)
		{
			setRecordData(argv[recordingID]);

			setMapFile(mapID == -1 ? firstMap : argv[mapID]);

			loadMap(mapID == -1 ? firstMap : argv[mapID], TRUE);
		}

		else if (replayingID != -1)
		{
			setReplayData(argv[replayingID], TRUE);

			loadMap(mapID == -1 ? firstMap : argv[mapID], TRUE);
		}

		else
		{
			loadMap(mapID == -1 ? firstMap : argv[mapID], TRUE);
		}
	}

	else
	{
		if (recordingID != -1)
		{
			game.gameType = RECORDING;
		}

		else if (replayingID != -1)
		{
			game.gameType = REPLAYING;
		}

		if (loadGame(loadSlot) == FALSE)
		{
			showErrorAndExit("No saved game in slot %d", loadSlot);
		}

		if (recordingID != -1)
		{
			setRecordData(argv[recordingID]);

			setMapFile(getMapFilename());
		}

		else if (replayingID != -1)
		{
			setReplayData(argv[replayingID], TRUE);
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
	
	frameLimit = SDL_GetTicks() + game.fps;
	
	printf("FPS is %d\n", game.fps);

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

			case IN_INVENTORY:
				doInventoryMenu();
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

		frameLimit = SDL_GetTicks() + game.fps;

		game.frames++;
	}

	/* Exit the program */

	exit(0);
}
