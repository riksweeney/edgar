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
#include "cursor.h"
#include "draw_editor.h"
#include "entity.h"
#include "graphics/decoration.h"
#include "graphics/graphics.h"
#include "init.h"
#include "input.h"
#include "map.h"
#include "system/resources.h"

Input input, menuInput;
Entity player, playerWeapon, playerShield, *self;
Game game;
Cursor cursor;
Control control;

int main(int argc, char *argv[])
{
	unsigned int frameLimit;
	int go, x, y, export, row, col, maxX, maxY;
	Entity startPos;
	char filename[MAX_VALUE_LENGTH];

	#if DEV == 0
		printf("Editor doesn't work if DEV is set to 0\n");

		exit(1);
	#endif

	setLanguage("edgar", NULL);

	/* Start up SDL */

	init("Map Editor", 1);

	/* Call the cleanup function when the program exits */

	atexit(cleanup);

	export = FALSE;

	go = TRUE;

	printf("This program is unsupported\n");

	/* Load the resources */

	loadRequiredResources();

	freeEntities();

	freeDecorations();

	if (argc > 1)
	{
		loadMap(argv[1], TRUE);

		if (argc > 2 && strcmpignorecase(argv[2], "EXPORT") == 0)
		{
			export = TRUE;
		}
	}

	else
	{
		printf("Usage: %s <map file name>\n", argv[0]);

		exit(1);
	}

	if (export == TRUE)
	{
		maxX = getMapMaxX();
		maxY = getMapMaxY();

		setMapMaxX(MAX_MAP_X * TILE_SIZE);
		setMapMaxY(MAX_MAP_Y * TILE_SIZE);
	}

	else
	{
		setMapMinX(0);
		setMapMaxY(0);

		setMapMaxX(MAX_MAP_X * TILE_SIZE);
		setMapMaxY(MAX_MAP_Y * TILE_SIZE);
	}

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

	game.fps = 1000 / 60;

	frameLimit = SDL_GetTicks() + game.fps;

	if (export == TRUE)
	{
		x = getMapMinX();
		y = getMapMinY();

		setMapStartX(x);
		setMapStartY(y);

		col = row = 0;

		while (go == TRUE)
		{
			/* Do the map */

			doMap();

			/* Draw the map */

			drawExport();

			snprintf(filename, MAX_VALUE_LENGTH, "%03d_%s_%03d.png", row, argv[1], col);

			takeSingleScreenshot(filename);

			x += SCREEN_WIDTH;

			col++;

			if (x >= maxX)
			{
				x = getMapMinX();

				row++;

				col = 0;

				y += SCREEN_HEIGHT;

				if (y >= maxY)
				{
					exit(0);
				}
			}

			setMapStartX(x);
			setMapStartY(y);
		}
	}

	else
	{
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

			frameLimit = SDL_GetTicks() + game.fps;
		}
	}

	/* Exit the program */

	exit(0);
}
