/*
Copyright (C) 2009-2019 Parallel Realities

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
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include "../headers.h"

#include "../audio/music.h"
#include "../graphics/font.h"
#include "../graphics/graphics.h"
#include "../input.h"

extern Game game;

static void drawError(void);

void showErrorAndExit(char *fmt, ...)
{
	int h, y;
	SDL_Rect dest;
	SDL_Surface *surface;
	Texture *targetTexture;
	Texture *title, *error1, *error2, *error3;
	char text[MAX_MESSAGE_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	error1 = error2 = error3 = title = NULL;

	if (game.status == IN_ERROR)
	{
		exit(1);
	}

	printf("%s\n", text);

	#if DEV == 1
		exit(1);
	#endif

	game.status = IN_ERROR;

	if (game.font != NULL)
	{
		surface = generateTextSurface(_("The Legend of Edgar has encountered the following error"), game.font, 0, 220, 0, 0, 0, 0);

		title = convertSurfaceToTexture(surface, TRUE);

		surface = generateTextSurface(text, game.font, 220, 220, 220, 0, 0, 0);

		error1 = convertSurfaceToTexture(surface, TRUE);

		surface = generateTextSurface(_("Please report this error to Parallel Realities"), game.font, 0, 220, 0, 0, 0, 0);

		error2 = convertSurfaceToTexture(surface, TRUE);

		surface = generateTextSurface(_("Press Escape to exit"), game.font, 0, 220, 0, 0, 0, 0);

		error3 = convertSurfaceToTexture(surface, TRUE);
	}

	if (game.tempSurface != NULL)
	{
		destroyTexture(game.tempSurface);

		game.tempSurface = NULL;
	}
	
	targetTexture = createWritableTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_SetRenderTarget(game.renderer, targetTexture->texture);

	drawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 255);

	h = title->h + error1->h + error2->h + error3->h + 45;

	y = (SCREEN_HEIGHT - h) / 2;

	dest.x = (SCREEN_WIDTH - title->w) / 2;
	dest.y = y;
	dest.w = title->w;
	dest.h = title->h;

	SDL_RenderCopy(game.renderer, title->texture, NULL, &dest);

	y += title->h + 15;

	dest.x = (SCREEN_WIDTH - error1->w) / 2;
	dest.y = y;
	dest.w = error1->w;
	dest.h = error1->h;

	SDL_RenderCopy(game.renderer, error1->texture, NULL, &dest);

	y += error1->h + 15;

	dest.x = (SCREEN_WIDTH - error2->w) / 2;
	dest.y = y;
	dest.w = error2->w;
	dest.h = error2->h;

	SDL_RenderCopy(game.renderer, error2->texture, NULL, &dest);

	y += error2->h + 15;

	dest.x = (SCREEN_WIDTH - error3->w) / 2;
	dest.y = y;
	dest.w = error3->w;
	dest.h = error3->h;

	SDL_RenderCopy(game.renderer, error3->texture, NULL, &dest);

	destroyTexture(title);
	destroyTexture(error1);
	destroyTexture(error2);
	destroyTexture(error3);
	
	game.tempSurface = targetTexture;

	SDL_SetRenderTarget(game.renderer, NULL);

	stopMusic();

	drawError();
}

static void drawError()
{
	do
	{
		getInput(game.gameType);

		clearScreen(0, 0, 0);

		SDL_RenderCopy(game.renderer, game.tempSurface->texture, NULL, NULL);

		/* Swap the buffers */

		SDL_RenderPresent(game.renderer);

		/* Sleep briefly */

		SDL_Delay(16);
	}

	while (TRUE);
}
