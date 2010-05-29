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

#include "../graphics/graphics.h"
#include "../graphics/font.h"
#include "label.h"
#include "../system/error.h"

extern Game game;

Widget *createWidget(char *text, int *controlValue, void (*leftAction)(void), void (*rightAction)(void), void (*clickAction)(void), int x, int y, int border)
{
	Widget *w;

	w = (Widget *)malloc(sizeof(Widget));

	if (w == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes to create Widget %s", (int)sizeof(Widget), text);
	}

	if (border == TRUE)
	{
		w->normalState = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 0, 0, 0), 255, 255, 255, 0, 0, 0);

		w->selectedState = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 0, 200, 0), 255, 255, 255, 0, 200, 0);
		
		w->disabledState = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 100, 100, 100), 255, 255, 255, 100, 100, 100);
	}

	else
	{
		w->normalState = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 0, 0, 0), 0, 0, 0, 0, 0, 0);

		w->selectedState = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 0, 200, 0), 0, 200, 0, 0, 200, 0);
		
		w->disabledState = addBorder(generateTextSurface(text, game.font, 255, 255, 255, 100, 100, 100), 0, 0, 0, 0, 0, 0);
	}

	w->value = controlValue;

	w->leftAction = leftAction;

	w->rightAction = rightAction;

	w->clickAction = clickAction;

	w->x = x;

	w->y = y;

	w->label = NULL;
	
	w->disabled = FALSE;

	return w;
}

void drawWidget(Widget *w, Menu *m, int selected)
{
	int x, y;

	x = w->x < 0 ? (m->w - w->normalState->w) / 2 : w->x;
	y = w->y < 0 ? (m->h - w->normalState->h) / 2 : w->y;

	x += m->x;
	y += m->y;
	
	if (w->disabled == TRUE)
	{
		drawImage(w->disabledState, x, y, FALSE, 255);
	}
	
	else
	{
		drawImage(selected == TRUE ? w->selectedState : w->normalState, x, y, FALSE, 255);
	}

	if (w->label != NULL)
	{
		drawLabel(w->label, m);
	}
}

void freeWidget(Widget *w)
{
	if (w != NULL)
	{
		if (w->normalState != NULL)
		{
			SDL_FreeSurface(w->normalState);

			w->normalState = NULL;
		}

		if (w->selectedState != NULL)
		{
			SDL_FreeSurface(w->selectedState);

			w->selectedState = NULL;
		}
		
		if (w->selectedState != NULL)
		{
			SDL_FreeSurface(w->disabledState);

			w->disabledState = NULL;
		}

		if (w->label != NULL)
		{
			freeLabel(w->label);
		}

		free(w);
	}
}
