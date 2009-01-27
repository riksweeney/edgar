#include "headers.h"

extern Game game;
extern Entity player;

void draw()
{
	char text[20];

	/* Clear the screen */

	SDL_FillRect(game.screen, NULL, 0);

	/* Center the map */

	centerEntityOnMap();

	/* Draw the map */

	drawMap();

	/* Draw the hud */

	drawHud();

	/* Draw the decorations */

	drawDecorations();

	/* Draw the Entities */

	drawEntities(0);

	/* Draw the player */

	drawPlayer();

	/* Draw the game statuses */

	drawGame();

	/* Draw the screen coordinates */

	if (player.face == LEFT)
	{
		sprintf(text, "%3d : %3d", (int)player.x, (int)player.y + player.h - 1);
	}

	else
	{
		sprintf(text, "%3d : %3d", (int)player.x + player.w - 1, (int)player.y + player.h - 1);
	}

	drawString(text, 5, 5, game.font, 0, 0);

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
