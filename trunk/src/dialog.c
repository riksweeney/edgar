/*
Copyright (C) 2009 Parallel Realities

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

#include "headers.h"

#include "graphics/graphics.h"
#include "graphics/font.h"
#include "dialog.h"

extern Game game;

static SDL_Surface *dialogSurface;

void createDialogBoxFromScript(char *msg)
{
	char *text, *title, *savePtr;

	title = strtok_r(msg, " ", &savePtr);

	title = strtok_r(NULL, " ", &savePtr);

	text = strtok_r(NULL, "\0", &savePtr);

	dialogSurface = createDialogBox(title, text);
}

SDL_Surface *createDialogBox(char *title, char *msg)
{
	char *text, *token, word[MAX_VALUE_LENGTH], *savePtr;
	int i, lines, w, h, maxWidth;
	SDL_Surface **surface, *tempSurface;
	SDL_Rect dest;

	freeDialogBox();

	text = (char *)malloc(strlen(msg) + 1);

	if (text == NULL)
	{
		printf("Could not allocate a whole %d bytes for the Dialog Text\n", strlen(msg) + 1);

		exit(1);
	}

	STRNCPY(text, msg, strlen(msg) + 1);

	token = strtok_r(text, " ", &savePtr);

	i = 0;

	while (token != NULL)
	{
		i++;

		token = strtok_r(NULL, " ", &savePtr);
	}

	lines = i;

	if (title != NULL)
	{
		lines++;
	}

	surface = (SDL_Surface **)malloc(sizeof(SDL_Surface *) * lines);

	if (surface == NULL)
	{
		printf("Could not allocate a whole %d bytes for the Dialog Surfaces\n", sizeof(SDL_Surface *) * lines);

		exit(1);
	}

	STRNCPY(text, msg, strlen(msg) + 1);

	token = strtok_r(text, " ", &savePtr);

	i = 0;

	maxWidth = w = h = 0;

	if (title != NULL)
	{
		surface[i] = generateTextSurface(title, game.font, 255, 255, 0, 0, 0, 0);

		h = surface[i]->h + 5;

		maxWidth = surface[i]->w;

		i++;
	}

	while (token != NULL)
	{
		snprintf(word, sizeof(word), "%s ", token);

		surface[i] = generateTextSurface(word, game.font, 255, 255, 255, 0, 0, 0);

		if (h == 0 || (i == 1 && title != NULL))
		{
			h += surface[i]->h + 5;
		}

		if (w + surface[i]->w > MAX_DIALOG_WIDTH)
		{
			w = 0;

			h += surface[i]->h + 5;
		}

		w += surface[i]->w;

		if (w > maxWidth)
		{
			maxWidth = w;
		}

		i++;

		token = strtok_r(NULL, " ", &savePtr);
	}

	h -= 5;

	tempSurface = createSurface(maxWidth, h);

	w = h = 0;

	for (i=0;i<lines;i++)
	{
		if (w + surface[i]->w > MAX_DIALOG_WIDTH || (title != NULL && i == 1))
		{
			w = 0;

			h += surface[i]->h + 5;
		}

		dest.x = w;
		dest.y = h;
		dest.w = surface[i]->w;
		dest.h = surface[i]->h;

		SDL_BlitSurface(surface[i], NULL, tempSurface, &dest);

		w += surface[i]->w;

		SDL_FreeSurface(surface[i]);
	}

	tempSurface = addBorder(tempSurface, 255, 255, 255, 0, 0, 0);

	free(surface);

	free(text);

	return tempSurface;
}

void drawDialogBox()
{
	if (dialogSurface != NULL)
	{
		drawImage(dialogSurface, (SCREEN_WIDTH - dialogSurface->w) / 2, 50, FALSE);
	}
}

void freeDialogBox()
{
	if (dialogSurface != NULL)
	{
		SDL_FreeSurface(dialogSurface);

		dialogSurface = NULL;
	}
}
