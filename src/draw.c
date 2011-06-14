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
#include "event/script.h"
#include "credits.h"

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
			case IN_INVENTORY:
				SDL_SetClipRect(game.screen, NULL);

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

						SDL_SetClipRect(game.screen, &rect);
					}

					else
					{
						SDL_SetClipRect(game.screen, NULL);
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

						SDL_SetClipRect(game.screen, NULL);
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

						drawString(text, 5, 30, game.font, 0, 0, 255, 255, 255);
					#endif
				}
			break;
		}

		#if DEV == 1
			takeScreenshot();
		#endif
	}

	else
	{
		SDL_SetClipRect(game.screen, NULL);

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

	if (frameLimit > ticks + game.fps)
	{
		SDL_Delay(game.fps);
	}

	else
	{
		SDL_Delay(frameLimit - ticks);
	}
}
