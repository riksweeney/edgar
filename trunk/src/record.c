#include "headers.h"

static int frame = 0;

extern Game game;

void takeScreenshot()
{
	char filename[MAX_PATH_LENGTH];

	sprintf(filename, "/home/rik/temp/edgar%06d.bmp", frame);

	frame++;

	SDL_SaveBMP(game.screen, filename);
}
