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

#include "cursor.h"
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
#include "system/load_save.h"

Input input, menuInput;
Entity player, playerWeapon, playerShield, *self, entity[MAX_ENTITIES];
Animation animation[MAX_ANIMATIONS];
Sound sound[MAX_SOUNDS];
Game game;
Inventory inventory;
Droplet droplet[MAX_DROPS];
Cursor cursor;
Target target[MAX_TARGETS];
Control control;

int main(int argc, char *argv[])
{
	unsigned int frameLimit = SDL_GetTicks() + 16;
	int go;
	Entity startPos;

	/* Start up SDL */

	init("Map Editor");

	/* Call the cleanup function when the program exits */

	atexit(cleanup);

	go = TRUE;

	printf("NO SUPPORT IS GIVEN FOR THIS PROGRAM, SO DON'T ASK HOW IT WORKS!\n");

	if (argc > 1)
	{
		loadMap(argv[1], TRUE);
	}

	else
	{
		printf("Usage: %s <map file name>\n", argv[0]);

		exit(1);
	}

	/* Load the resources */

	loadRequiredResources();

	/* Load the background image */
	
	setMinMapX(0);
	setMinMapY(0);

	setMaxMapX(MAX_MAP_X * TILE_SIZE);
	setMaxMapY(MAX_MAP_Y * TILE_SIZE);

	/* Initialise the cursor */

	initCursor(argv[1]);

	/* Initialise the line defs */

	initLineDefs();

	/* Loop indefinitely for messages */

	startPos.x = (int)(player.x / TILE_SIZE);
	startPos.y = (int)(player.y / TILE_SIZE);

	startPos.x *= TILE_SIZE;
	startPos.y *= TILE_SIZE;

	startPos.x -= 9 * TILE_SIZE;
	startPos.y -= 7 * TILE_SIZE;

	if (startPos.x < 0)
	{
		startPos.x = 0;
	}

	if (startPos.y < 0)
	{
		startPos.y = 0;
	}

	setMapStartX(startPos.x);
	setMapStartY(startPos.y);

	resetControls(TRUE);
	
	game.status = IN_EDITOR;

	while (go == TRUE)
	{
		/* Get the input */

		getInput(game.gameType);

		/* Do the cursor */

		doCursor();

		/* Do the map */

		doMap();

		/* Draw the map */

		draw();

		/* Sleep briefly to stop sucking up all the CPU time */

		delay(frameLimit);

		frameLimit = SDL_GetTicks() + 16;
	}

	/* Exit the program */

	exit(0);
}
