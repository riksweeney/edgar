/*
Copyright (C) 2009-2010 Parallel Realities

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

#include "../headers.h"
#include "../graphics/font.h"
#include "../graphics/graphics.h"
#include "../audio/music.h"
#include "../input.h"

extern Game game;

static void drawError(void);

void showErrorAndExit(char *fmt, ...)
{
	int h, y;
	SDL_Rect src, dest;
	SDL_Surface *title, *error1, *error2, *error3;
	char text[MAX_MESSAGE_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	title = generateTextSurface(_("The Legend of Edgar has encountered the following error"), game.font, 0, 220, 0, 0, 0, 0);

	error1 = generateTextSurface(text, game.font, 220, 220, 220, 0, 0, 0);

	error2 = generateTextSurface(_("Please report this error to Parallel Realities"), game.font, 0, 220, 0, 0, 0, 0);

	error3 = generateTextSurface(_("The game will now exit"), game.font, 0, 220, 0, 0, 0, 0);

	printf("%s\n", text);

	#if DEV == 1
		exit(1);
	#endif

	if (title == NULL || error1 == NULL || error2 == NULL || error3 == NULL)
	{
		exit(1);
	}

	if (game.tempSurface != NULL)
	{
		SDL_FreeSurface(game.tempSurface);

		game.tempSurface = NULL;
	}

	game.tempSurface = createSurface(SCREEN_WIDTH, SCREEN_HEIGHT);

	drawBox(game.tempSurface, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);

	h = title->h + error1->h + error2->h + error3->h + 45;

	y = (SCREEN_HEIGHT - h) / 2;

	src.x = 0;
	src.y = 0;
	src.w = title->w;
	src.h = title->h;

	dest.x = (SCREEN_WIDTH - title->w) / 2;
	dest.y = y;
	dest.w = title->w;
	dest.h = title->h;

	SDL_BlitSurface(title, &src, game.tempSurface, &dest);

	y += title->h + 15;

	src.x = 0;
	src.y = 0;
	src.w = error1->w;
	src.h = error1->h;

	dest.x = (SCREEN_WIDTH - error1->w) / 2;
	dest.y = y;
	dest.w = error1->w;
	dest.h = error1->h;

	SDL_BlitSurface(error1, &src, game.tempSurface, &dest);

	y += error1->h + 15;

	src.x = 0;
	src.y = 0;
	src.w = error2->w;
	src.h = error2->h;

	dest.x = (SCREEN_WIDTH - error2->w) / 2;
	dest.y = y;
	dest.w = error2->w;
	dest.h = error2->h;

	SDL_BlitSurface(error2, &src, game.tempSurface, &dest);

	y += error2->h + 15;

	src.x = 0;
	src.y = 0;
	src.w = error3->w;
	src.h = error3->h;

	dest.x = (SCREEN_WIDTH - error3->w) / 2;
	dest.y = y;
	dest.w = error3->w;
	dest.h = error3->h;

	SDL_BlitSurface(error3, &src, game.tempSurface, &dest);

	SDL_FreeSurface(title);
	SDL_FreeSurface(error1);
	SDL_FreeSurface(error2);
	SDL_FreeSurface(error3);

	game.status = IN_ERROR;

	stopMusic();

	drawError();
}

static void drawError()
{
	do
	{
		getInput(game.gameType);

		clearScreen(0, 0, 0);

		SDL_BlitSurface(game.tempSurface, NULL, game.screen, NULL);

		/* Swap the buffers */

		SDL_Flip(game.screen);

		/* Sleep briefly */

		SDL_Delay(16);
	}

	while (TRUE);
}
