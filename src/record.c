#include "record.h"

static int frame = 0;

void takeScreenshot()
{
	char filename[MAX_LINE_LENGTH];

	sprintf(filename, "/home/rik/temp/edgar%06d.bmp", frame);
	
	frame++;
	
	SDL_SaveBMP(game.screen, filename);
}
