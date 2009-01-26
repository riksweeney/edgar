#include "draw_editor.h"

extern void drawMap(void);
extern void drawCursor(void);
extern void drawStatusPanel(void);
extern void drawEntities(void);
extern void drawPlayer(void);
extern void drawString(char *, int, int, TTF_Font *, int, int);
extern int mapStartX(void);
extern int mapStartY(void);
extern void drawTargets(void);

void draw()
{
	char text[20];
	
	/* Draw the map */
	
	drawMap();
	
	/* Draw the entities */
	
	drawEntities();
	
	/* Draw the targets */
	
	drawTargets();
	
	/* Draw the cursor */
	
	drawCursor();
	
	/* Draw the player */
	
	drawPlayer();
	
	/* Draw the status panel */
	
	drawStatusPanel();
	
	/* Draw the screen coordinates */
	
	sprintf(text, "%5d : %5d", mapStartX() + cursor.x, mapStartY() + cursor.y);
	
	drawString(text, 0, 5, game.font, 1, 0);

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
