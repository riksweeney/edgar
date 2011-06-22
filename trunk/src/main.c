/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "medal.h"
#include "dialog.h"
#include "system/record.h"
#include "system/load_save.h"
#include "system/error.h"
#include "dialog.h"
#include "menu/menu.h"
#include "menu/inventory_menu.h"
#include "system/error.h"
#include "credits.h"

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
	int joystick, showCredits;

	#ifndef NO_GETTEXT
		printf("Locale is %s\n", setlocale(LC_ALL, ""));
		printf("Numeric is %s\n", setlocale(LC_NUMERIC, "C"));
		printf("atof(2.75) is %f\n", atof("2.75"));
		textdomain("edgar");
		bindtextdomain("edgar", LOCALE_DIR);
	#endif

	/* Call the cleanup function when the program exits */

	atexit(cleanup);

	go = TRUE;

	loadSlot = -1;

	game.fps = 1000 / 60;

	mapID = recordingID = replayingID = -1;
	
	joystick = 1;
	
	showCredits = 0;

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

		else if (strcmpignorecase("-nojoystick", argv[i]) == 0)
		{
			game.disableJoystick = TRUE;
		}
		
		else if (strcmpignorecase("-joystick", argv[i]) == 0)
		{
			joystick = atoi(argv[i + 1]);
			
			i++;
		}
		
		else if (strcmpignorecase("-showcredits", argv[i]) == 0)
		{
			showCredits = TRUE;
		}

		#if DEV == 1
			else if (strcmpignorecase("-saveonexit", argv[i]) == 0)
			{
				game.saveOnExit = TRUE;
			}

			else
			{
				mapID = i;
			}
		#endif
	}

	/* Start up SDL */

	init(_("The Legend of Edgar"), joystick);

	if (replayingID != -1 && recordingID != -1)
	{
		showErrorAndExit("Cannot record and replay at the same time");
	}

	loadRequiredResources();

	if (game.medalSupport == TRUE)
	{
		showMedalScreen();
	}

	/* Initialise the game variables */

	initGame();

	if (loadSlot == -1)
	{
		firstMap = game.firstRun == TRUE ? "tutorial" : "map01";

		if (recordingID != -1)
		{
			setRecordData(argv[recordingID]);

			setMapFile(mapID == -1 ? firstMap : argv[mapID]);
		}

		else if (replayingID != -1)
		{
			setReplayData(argv[replayingID], TRUE);
		}

		if (mapID != -1)
		{
			startOnMap(argv[mapID]);
		}
		
		else if (game.firstRun == TRUE)
		{
			tutorial();
		}
		
		else
		{
			newGame();
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

	/* Loop indefinitely for messages */

	game.startTicks = SDL_GetTicks();

	#if DEV == 1
		printf("DEV Version\n");
	#else
		printf("Production Version\n");
	#endif

	frameLimit = SDL_GetTicks() + game.fps;
	
	if (showCredits == TRUE)
	{
		game.status = IN_CREDITS;
	}

	while (go == TRUE)
	{
		getInput(game.gameType);

		switch (game.status)
		{
			case IN_GAME:
				freeCollisionGrid();

				clearDrawLayers();

				doGame();

				doPlayer();

				doInventory();

				doMap();

				doEntities();

				doDecorations();

				doCollisions();

				doHud();

				doDialogBox();

				processMedals();
			break;

			case IN_INVENTORY:
				doInventoryMenu();
			break;

			case IN_MENU:
				doMenu();
			break;
			
			case IN_CREDITS:
				freeCollisionGrid();
				
				clearDrawLayers();
				
				doGame();
				
				doCredits();
				
				doDecorations();
				
				doCollisions();
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
