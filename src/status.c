#include "headers.h"

#include "font.h"

extern Game game;

static Message message;

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
		drawString(message.text, 0, SCREEN_HEIGHT - TILE_SIZE, game.font, 1, 0);
	}
}

void setStatusMessage(char *text)
{
	strncpy(message.text, text, MAX_MESSAGE_LENGTH);

	message.thinkTime = 120;
}
