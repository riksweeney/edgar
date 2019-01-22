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

#include "credits.h"
#include "dialog.h"
#include "entity.h"
#include "event/script.h"
#include "game.h"
#include "graphics/decoration.h"
#include "graphics/graphics.h"
#include "graphics/texture_cache.h"
#include "hud.h"
#include "inventory.h"
#include "map.h"
#include "player.h"
#include "title.h"

extern Game game;
extern Entity player;

void draw()
{
	#if DEV == 1
		char text[MAX_VALUE_LENGTH];
	#endif
	int clipX, clipY;
	SDL_Rect rect;

	clipX = -1;
	clipY = -1;

	/* Clear the screen */

	if (game.paused == FALSE)
	{
		clearScreen(0, 0, 0);

		switch (game.status)
		{
			case IN_TITLE:
				drawTitle();
			break;

			case IN_INVENTORY:
				SDL_RenderSetClipRect(game.renderer, NULL);

				showPauseDialog();

				drawHud();

				drawInventory();
			break;

			case IN_CREDITS:
				drawEntities(BACKGROUND_LAYER);

				drawMap(1);

				drawEntities(MID_GROUND_LAYER);

				drawPlayer();

				drawDecorations();

				drawEntities(FOREGROUND_LAYER);

				drawCredits();

				drawGame();
			break;

			default:
				if (game.drawScreen == TRUE)
				{
					/* Center the map */

					centerEntityOnMap();

					if (isDarkMap() == TRUE)
					{
						/* Set the clipping bounds */

						clipX = player.x + player.w / 2 - spotlightSize() / 2 - getMapStartX();
						clipY = player.y + player.h / 2 - spotlightSize() / 2 - getMapStartY();

						rect.x = clipX;
						rect.y = clipY;

						rect.w = rect.h = spotlightSize();

						SDL_RenderSetClipRect(game.renderer, &rect);
					}

					else
					{
						SDL_RenderSetClipRect(game.renderer, NULL);
					}

					/* Draw the background */

					drawMapBackground();

					/* Draw the weather */

					drawWeather();

					/* Draw the background map tiles */

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

					/* Draw the Entities that appear in the foreground */

					drawEntities(FOREGROUND_LAYER);

					/* Draw the foreground map tiles */

					drawMap(2);

					if (isDarkMap() == TRUE)
					{
						/* Draw the spotlight */

						drawSpotlight(clipX, clipY);

						SDL_RenderSetClipRect(game.renderer, NULL);
					}

					/* Draw the hud */

					drawHud();

					/* Draw the dialog box */

					drawDialogBox();

					/* Draw the script dialog box */

					scriptDrawMenu();

					/* Draw the game statuses */

					drawGame();

					#if DEV == 1
						/* Draw the screen coordinates */

						if (player.face == LEFT)
						{
							snprintf(text, sizeof(text), "%3d : %3d", (int)player.x, (int)player.y);
						}

						else
						{
							snprintf(text, sizeof(text), "%3d : %3d", (int)player.x, (int)player.y);
						}
					#endif

					if (scriptRunning() == FALSE && (game.frames % 600 == 0))
					{
						checkTextureCache();
					}
				}
			break;
		}
	}

	else
	{
		SDL_RenderSetClipRect(game.renderer, NULL);

		showPauseDialog();

		game.drawMenu();
	}

	/* Swap the buffers */

	SDL_RenderPresent(game.renderer);

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

	if (frameLimit > ticks + game.fps)
	{
		SDL_Delay(game.fps);
	}

	else
	{
		SDL_Delay(frameLimit - ticks);
	}
}
