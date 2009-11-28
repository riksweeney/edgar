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

#include "map.h"
#include "graphics/decoration.h"
#include "entity.h"
#include "player.h"
#include "hud.h"
#include "game.h"
#include "graphics/font.h"
#include "system/record.h"
#include "system/error.h"
#include "graphics/graphics.h"
#include "dialog.h"
#include "inventory.h"

extern Game game;
extern Entity player;

void draw()
{
	char text[MAX_VALUE_LENGTH];

	/* Clear the screen */

	if (game.paused == FALSE)
	{
		clearScreen(0, 0, 0);

		switch (game.status)
		{
			case IN_INVENTORY:
				showPauseDialog();

				drawHud();

				drawInventory();
			break;

			default:
				if (game.drawScreen == TRUE)
				{
					/* Center the map */

					centerEntityOnMap();

					/* Draw the map background */

					drawMap(0);

					/* Draw the Entities */

					drawEntities(BACKGROUND_LAYER);

					/* Draw the mid-ground map tiles */

					drawMap(1);

					/* Draw the Entities that appear in the mid-ground */

					drawEntities(MID_GROUND_LAYER);

					/* Draw the player */

					drawPlayer();

					/* Draw the decorations */

					drawDecorations();

					/* Draw the foreground map tiles */

					drawMap(2);

					/* Draw the Entities that appear in the foreground */

					drawEntities(FOREGROUND_LAYER);

					/* Draw the hud */

					drawHud();

					/* Draw the dialog box */

					drawDialogBox();

					/* Draw the game statuses */

					drawGame();

					/* Draw the screen coordinates */

					if (player.face == LEFT)
					{
						snprintf(text, sizeof(text), "%3d : %3d", (int)player.x, (int)player.y);
					}

					else
					{
						snprintf(text, sizeof(text), "%3d : %3d", (int)player.x, (int)player.y);
					}

					/*drawString(text, 5, 5, game.font, 0, 0, 255, 255, 255);*/
				}
			break;
		}

		takeScreenshot();
	}

	else
	{
		showPauseDialog();

		game.drawMenu();
	}

	/* Swap the buffers */

	SDL_Flip(game.screen);

	/* Sleep briefly */

	SDL_Delay(1);
}

void delay(unsigned int frameLimit)
{
	unsigned int ticks = SDL_GetTicks();

	if (frameLimit < ticks)
	{
		return;
	}

	if (frameLimit > ticks + 16)
	{
		SDL_Delay(16);
	}

	else
	{
		SDL_Delay(frameLimit - ticks);
	}
}
