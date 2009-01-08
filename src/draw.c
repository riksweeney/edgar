#include "draw.h"

extern void drawPlayer(void);
extern void drawGame(void);
extern void drawEntities(void);
extern void drawMap(void);
extern void centerEntityOnMap(void);
extern void drawHud(void);
extern void drawDecorations(void);

void draw()
{
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
	
	drawEntities();

	/* Draw the player */
	
	drawPlayer();
	
	/* Draw the game statuses */
	
	drawGame();

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
