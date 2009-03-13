#include "headers.h"

#include "map.h"
#include "decoration.h"
#include "entity.h"
#include "player.h"
#include "hud.h"
#include "game.h"
#include "font.h"
#include "record.h"
#include "graphics.h"
#include "save_png.h"
#include "dialog.h"

extern Game game;
extern Entity player;

void draw()
{
	char text[MAX_VALUE_LENGTH];

	/* Clear the screen */
	
	if (game.paused == FALSE)
	{
		clearScreen(0, 0, 0);
	
		if (game.drawScreen == TRUE)
		{
			/* Center the map */
	
			centerEntityOnMap();
	
			/* Draw the mid-ground map tiles */
	
			drawMap(0);
	
			/* Draw the Entities */
	
			drawEntities(0);
	
			/* Draw the decorations */
	
			drawDecorations();
	
			/* Draw the player */
	
			drawPlayer();
	
			/* Draw the foreground map tiles */
	
			drawMap(1);
	
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
	
		takeScreenshot();
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
