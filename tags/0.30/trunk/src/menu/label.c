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

#include "../headers.h"

#include "../graphics/graphics.h"
#include "../graphics/font.h"

extern Game game;

Label *createLabel(char *text, int x, int y)
{
	Label *l;

	l = (Label *)malloc(sizeof(Label));

	if (l == NULL)
	{
		printf("Failed to allocate %d bytes to create Label %s\n", (int)sizeof(Label), text);

		exit(1);
	}

	l->text = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 0, 0, 0), 255, 255, 255, 0, 0, 0);

	l->x = x;

	l->y = y;

	return l;
}

void updateLabelText(Label *l, char *text)
{
	if (l->text != NULL)
	{
		SDL_FreeSurface(l->text);
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
		drawImage(l->text, x, y, FALSE, 255);
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
	}
}
