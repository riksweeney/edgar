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

#include "../graphics/font.h"
#include "../graphics/graphics.h"
#include "../graphics/texture_cache.h"
#include "../system/error.h"

extern Game game;

Label *createLabel(char *text, int x, int y)
{
	Label *l;
	char keyName[MAX_FILE_LENGTH];
	SDL_Surface *textSurface;

	snprintf(keyName, MAX_FILE_LENGTH, "l_%s", text);

	l = malloc(sizeof(Label));

	if (l == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes to create Label %s", (int)sizeof(Label), text);
	}

	l->text = getTextureFromCache(keyName);

	if (l->text == NULL)
	{
		textSurface = generateTextSurface(text, game.font, 255, 255, 255, 0, 0, 0);
		
		l->text = addBorder(textSurface, 255, 255, 255, 0, 0, 0);

		addTextureToCache(keyName, l->text, TRUE);
	}

	l->x = x;

	l->y = y;

	return l;
}

Label *createImageLabel(Texture *texture, int x, int y)
{
	Label *l;

	l = malloc(sizeof(Label));

	if (l == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes to create image Label", (int)sizeof(Label));
	}

	l->text = texture;

	l->x = x;

	l->y = y;

	return l;
}

void updateLabelText(Label *l, char *text)
{
	char keyName[MAX_VALUE_LENGTH];

	snprintf(keyName, MAX_VALUE_LENGTH, "l_%s", text);

	l->text = getTextureFromCache(keyName);

	if (l->text == NULL)
	{
		l->text = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 0, 0, 0), 255, 255, 255, 0, 0, 0);

		addTextureToCache(keyName, l->text, TRUE);
	}
}

void drawLabel(Label *l, Menu *m)
{
	int x, y;

	if (l != NULL)
	{
		x = l->x < 0 ? (m->w - l->text->w) / 2 : l->x;
		y = l->y < 0 ? (m->h - l->text->h) / 2 : l->y;

		x += m->x;
		y += m->y;

		drawImage(l->text, x, y - m->startY, FALSE, 255);
	}
}

void freeLabel(Label *l)
{
	if (l != NULL)
	{
		free(l);

		l = NULL;
	}
}
