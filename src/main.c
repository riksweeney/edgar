/*
Copyright (C) 2009-2019 Parallel Realities

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
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include "headers.h"

#include "collisions.h"
#include "credits.h"
#include "dialog.h"
#include "draw.h"
#include "entity.h"
#include "event/script.h"
#include "game.h"
#include "graphics/decoration.h"
#include "graphics/graphics.h"
#include "graphics/texture_cache.h"
#include "hud.h"
#include "init.h"
#include "input.h"
#include "inventory.h"
#include "map.h"
#include "medal.h"
#include "menu/inventory_menu.h"
#include "menu/menu.h"
#include "player.h"
#include "system/error.h"
#include "system/load_save.h"
#include "system/pak.h"
#include "system/record.h"
#include "system/resources.h"
#include "title.h"

Input input, menuInput;
Entity *self;
Game game;
Entity player, playerShield, playerWeapon;
Control control;

int main(int argc, char *argv[])
{
	unsigned int frameLimit;
	int go, i, mapID, loadSlot, recordingID, replayingID;
	int joystick, showCredits, languageID;

	go = TRUE;

	loadSlot = -1;

	game.fps = 1000 / 60;

	languageID = mapID = recordingID = replayingID = -1;

	joystick = 0;

	showCredits = FALSE;

	/* Load the resources */

	for (i=1;i<argc;i++)
	{
		if (strcmpignorecase("-record", argv[i]) == 0)
		{
			if (i + 1 >= argc)
			{
				printf("You must specify a file to record to\n");
				printf("Type %s -h for help\n", argv[0]);

				exit(1);
			}

			if (recordingID == -1)
			{
				recordingID = i + 1;
			}

			i++;
		}

		else if (strcmpignorecase("-playback", argv[i]) == 0)
		{
			if (i + 1 >= argc)
			{
				printf("You must specify a file to playback from\n");
				printf("Type %s -h for help\n", argv[0]);

				exit(1);
			}

			if (replayingID == -1)
			{
				replayingID = i + 1;
			}

			i++;
		}

		else if (strcmpignorecase("-load", argv[i]) == 0)
		{
			if (i + 1 >= argc)
			{
				printf("You must specify a slot to load from\n");
				printf("Type %s -h for help\n", argv[0]);

				exit(1);
			}

			loadSlot = atoi(argv[i + 1]);

			i++;
		}

		else if (strcmpignorecase("-nojoystick", argv[i]) == 0)
		{
			game.disableJoystick = TRUE;
		}

		else if (strcmpignorecase("-joystick", argv[i]) == 0)
		{
			if (i + 1 >= argc)
			{
				printf("You must specify a joystick slot to use\n");
				printf("Type %s -h for help\n", argv[0]);

				exit(1);
			}

			joystick = atoi(argv[i + 1]);

			i++;
		}

		else if (strcmpignorecase("-showcredits", argv[i]) == 0)
		{
			showCredits = TRUE;
		}

		else if (strstr(argv[i], "-lang") != NULL)
		{
			if (i + 1 >= argc)
			{
				printf("You must specify a language to use\n");
				printf("Type %s -h for help\n", argv[0]);

				exit(1);
			}

			languageID = i + 1;

			i++;
		}

		else if (strstr(argv[i], "-h") != NULL || strstr(argv[i], "-help") != NULL)
		{
			printf("The Legend of Edgar options\n\n");
			printf("\t-record <filename>: Captures keyboard input\n");
			printf("\t-playback <filename>: Replays keyboard input\n");
			printf("\t-load <save_slot>: Loads the game in slot <save_slot>. Slots start at 0\n");
			printf("\t-nojoystick: Disables the joystick\n");
			printf("\t-joystick <joystick_slot>: Use joystick <joystick_slot>. Slots start at 0\n");
			printf("\t-showcredits: Shows the end credits\n");
			printf("\t-language <language_code>: Use language <language_code>. e.g. en_US, es, pl\n\n");

			exit(0);
		}

		#if DEV == 1
			else if (strcmpignorecase("-saveonexit", argv[i]) == 0)
			{
				game.saveOnExit = TRUE;
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
		#endif
	}

	setLanguage("edgar", languageID == -1 ? NULL : argv[languageID]);
	printf("Numeric is %s\n", setlocale(LC_NUMERIC, "C"));
	printf("atof(2.75) is %f\n", atof("2.75"));

	/* Call the cleanup function when the program exits */

	atexit(cleanup);

	/* Start up SDL */

	init(_("The Legend of Edgar"), joystick);

	loadRequiredResources();

	if (replayingID != -1 && recordingID != -1)
	{
		showErrorAndExit("Cannot record and replay at the same time");
	}

	#if DEV == 0
		verifyVersion();
	#endif

	/* Initialise the game variables */

	freeGameResources();

	initGame();

	if (loadSlot == -1)
	{
		if (recordingID != -1)
		{
			setRecordData(argv[recordingID]);

			setMapFile(mapID == -1 ? "map01" : argv[mapID]);
		}

		else if (replayingID != -1)
		{
			setReplayData(argv[replayingID], TRUE);

			setMapFile(mapID == -1 ? "map01" : argv[mapID]);
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
			game.status = IN_TITLE;
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
			case IN_TITLE:
				doTitle();
			break;

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
