/*
Copyright (C) 2009-2012 Parallel Realities

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

#include "../graphics/font.h"
#include "../graphics/graphics.h"
#include "../system/error.h"

extern Game game;

Label *createLabel(char *text, int x, int y)
{
	Label *l;

	l = malloc(sizeof(Label));

	if (l == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes to create Label %s", (int)sizeof(Label), text);
	}

	l->text = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 0, 0, 0), 255, 255, 255, 0, 0, 0);

	l->x = x;

	l->y = y;

	return l;
}

Label *createImageLabel(SDL_Surface *image, int x, int y)
{
	Label *l;

	l = malloc(sizeof(Label));

	if (l == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes to create image Label", (int)sizeof(Label));
	}

	l->text = copyImage(image, 0, 0, image->w, image->h);

	l->x = x;

	l->y = y;

	return l;
}

void updateLabelText(Label *l, char *text)
{
	if (l->text != NULL)
	{
		SDL_FreeSurface(l->text);

		l->text = NULL;
	}

	l->text = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 0, 0, 0), 255, 255, 255, 0, 0, 0);
}

void drawLabel(Label *l, Menu *m)
{
	int x, y;

	x = l->x < 0 ? (m->w - l->text->w) / 2 : l->x;
	y = l->y < 0 ? (m->h - l->text->h) / 2 : l->y;

	x += m->x;
	y += m->y;

	if (l != NULL)
	{
		drawImage(l->text, x, y - m->startY, FALSE, 255);
	}
}

void freeLabel(Label *l)
{
	if (l != NULL)
	{
		if (l->text != NULL)
		{
			SDL_FreeSurface(l->text);

			l->text = NULL;
		}

		free(l);

		l = NULL;
	}
}
