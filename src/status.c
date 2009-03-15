#include "headers.h"

#include "graphics/font.h"

extern Game game;

/*
void doStatusPanel()
{
	message.thinkTime--;

	if (message.thinkTime <= 0)
	{
		message.thinkTime = 0;
	}
}

void drawStatusPanel()
{
	SDL_Rect dest;

	dest.x = 0;
	dest.y = SCREEN_HEIGHT - TILE_SIZE;
	dest.w = SCREEN_WIDTH;
	dest.h = TILE_SIZE;

	SDL_FillRect(game.screen, &dest, 0);

	if (message.thinkTime > 0)
	{
		drawString(message.text, 0, SCREEN_HEIGHT - TILE_SIZE, game.font, 1, 0, 255, 255, 255);
	}
}

void setStatusMessage(char *text)
{
	STRNCPY(message.text, text, sizeof(message.text));

	message.thinkTime = 120;
}
*/
