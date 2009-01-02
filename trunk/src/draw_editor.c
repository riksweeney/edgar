#include "draw_editor.h"

extern void drawMap(void);
extern void drawCursor(void);
extern void drawStatusPanel(void);
extern void drawEntities(void);
extern void drawPlayer(void);

void draw()
{
	/* Draw the map */
	
	drawMap();
	
	/* Draw the status panel */
	
	drawStatusPanel();
	
	/* Draw the entities */
	
	drawEntities();
	
	/* Draw the cursor */
	
	drawCursor();
	
	/* Draw the player */
	
	drawPlayer();

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
