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

#include "cursor.h"
#include "entity.h"
#include "graphics/font.h"
#include "graphics/graphics.h"
#include "map.h"
#include "player.h"
#include "status_panel.h"
#include "world/target.h"

extern Game game;
extern Cursor cursor;

void draw()
{
	char text[MAX_VALUE_LENGTH];

	/* Clear screen */

	clearScreen(0, 0, 0);

	/* Draw the background */

	drawMapBackground();

	/* Draw the map whole map */

	drawMap(-1);

	/* Draw the entities */

	drawEntities(-1);

	/* Draw the targets */

	drawTargets();

	/* Draw the cursor */

	drawCursor();

	/* Draw the player */

	drawPlayer();

	/* Draw the status panel */

	drawStatusPanel();

	/* Draw the screen coordinates */

	snprintf(text, sizeof(text), "%5d : %5d", getMapStartX() + cursor.x, getMapStartY() + cursor.y);

	drawString(text, 0, 5, game.font, 1, 0, 255, 255, 255);

	/* Swap the buffers */

	SDL_RenderPresent(game.renderer);

	/* Sleep briefly */

	SDL_Delay(1);
}

void drawExport()
{
	/* Clear screen */

	clearScreen(0, 0, 0);

	/* Draw the map whole map */

	drawMap(-1);

	/* Draw the entities */

	drawEntities(-1);

	/* Swap the buffers */

	SDL_RenderPresent(game.renderer);
}

void delay(unsigned int frameLimit)
{
	unsigned int ticks = SDL_GetTicks();

	if (frameLimit < ticks)
	{
		return;
	}

	if (frameLimit > ticks + game.fps)
	{
		SDL_Delay(game.fps);
	}

	else
	{
		SDL_Delay(frameLimit - ticks);
	}
}
