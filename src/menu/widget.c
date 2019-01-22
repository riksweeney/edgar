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
#include "../system/error.h"
#include "label.h"

extern Game game;

static SDL_Surface *createWidgetText(char *, TTF_Font *, int, int, int, int, int, int);

Widget *createWidget(char *text, int *controlValue, void (*leftAction)(void), void (*rightAction)(void), void (*clickAction)(void), int x, int y, int border, int r, int g, int b)
{
	Widget *w;

	w = malloc(sizeof(Widget));

	if (w == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes to create Widget %s", (int)sizeof(Widget), text);
	}

	if (border == TRUE)
	{
		w->normalState = addBorder(createWidgetText(text, game.font, r, g, b, 0, 0, 0), 255, 255, 255, 0, 0, 0);

		w->selectedState = addBorder(createWidgetText(text, game.font, r, g, b, 0, 200, 0), 255, 255, 255, 0, 200, 0);

		w->disabledState = addBorder(createWidgetText(text, game.font, r, g, b, 100, 100, 100), 255, 255, 255, 100, 100, 100);
	}

	else
	{
		w->normalState = addBorder(createWidgetText(text, game.font, r, g, b, 0, 0, 0), 0, 0, 0, 0, 0, 0);

		w->selectedState = addBorder(createWidgetText(text, game.font, r, g, b, 0, 200, 0), 0, 200, 0, 0, 200, 0);

		w->disabledState = addBorder(createWidgetText(text, game.font, r, g, b, 100, 100, 100), 0, 0, 0, 0, 0, 0);
	}

	w->value = controlValue;

	w->leftAction = leftAction;

	w->rightAction = rightAction;

	w->clickAction = clickAction;

	w->x = x;

	w->y = y;

	w->label = NULL;

	w->disabled = FALSE;

	w->hidden = FALSE;

	return w;
}

void drawWidget(Widget *w, Menu *m, int selected)
{
	int x, y;

	if (w->hidden == TRUE)
	{
		return;
	}

	x = w->x < 0 ? (m->background->w - w->normalState->w) / 2 : w->x;
	y = w->y < 0 ? (m->background->h - w->normalState->h) / 2 : w->y;

	x += m->x;
	y += m->y;

	if (w->disabled == TRUE)
	{
		drawImage(w->disabledState, x, y - m->startY, FALSE, 255);
	}

	else
	{
		drawImage(selected == TRUE ? w->selectedState : w->normalState, x, y - m->startY, FALSE, 255);
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
			destroyTexture(w->normalState);

			w->normalState = NULL;
		}

		if (w->selectedState != NULL)
		{
			destroyTexture(w->selectedState);

			w->selectedState = NULL;
		}

		if (w->disabledState != NULL)
		{
			destroyTexture(w->disabledState);

			w->disabledState = NULL;
		}

		if (w->label != NULL)
		{
			freeLabel(w->label);

			w->label = NULL;
		}

		free(w);

		w = NULL;
	}
}

static SDL_Surface *createWidgetText(char *msg, TTF_Font *font, int fr, int fg, int fb, int br, int bg, int bb)
{
	char *text, *token, word[MAX_VALUE_LENGTH], *savePtr;
	int i, lines, w, h, maxWidth, lineBreak, *lineBreaks;
	SDL_Surface **surface, *tempSurface;
	SDL_Rect dest;

	savePtr = NULL;

	text = malloc(strlen(msg) + 1);

	if (text == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for the Dialog Text", (int)strlen(msg) + 1);
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

	surface = malloc(sizeof(SDL_Texture *) * lines);

	if (surface == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for the Dialog Surfaces", (int)sizeof(SDL_Texture *) * lines);
	}

	lineBreaks = malloc(sizeof(int) * lines);

	if (lineBreaks == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for the line breaks", (int)sizeof(int) * lines);
	}

	STRNCPY(text, msg, strlen(msg) + 1);

	token = strtok_r(text, " ", &savePtr);

	i = 0;

	maxWidth = w = h = 0;

	while (token != NULL)
	{
		lineBreak = FALSE;

		snprintf(word, sizeof(word), "%s ", token);

		if (word[strlen(word) - 2] == '\n')
		{
			lineBreak = TRUE;

			word[strlen(word) - 2] = '\0';
		}

		token = strtok_r(NULL, " ", &savePtr);

		if (token == NULL)
		{
			word[strlen(word) - 1] = '\0';
		}

		surface[i] = generateTextSurface(word, game.font, fr, fg, fb, br, bg, bb);

		lineBreaks[i] = lineBreak;

		if (h == 0)
		{
			h += surface[i]->h + 5;
		}

		if (w + surface[i]->w > MAX_SCRIPT_WIDTH)
		{
			w = 0;

			h += surface[i]->h + 5;
		}

		w += surface[i]->w;

		if (w > maxWidth)
		{
			maxWidth = w;
		}

		if (lineBreak == TRUE)
		{
			w = 0;

			h += surface[i]->h + 5;
		}

		i++;
	}

	h -= 5;

	tempSurface = createSurface(maxWidth, h, FALSE);

	w = h = 0;

	for (i=0;i<lines;i++)
	{
		if (w + surface[i]->w > MAX_SCRIPT_WIDTH)
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

		if (lineBreaks[i] == TRUE)
		{
			w = 0;

			h += surface[i]->h + 5;
		}
	}

	free(surface);

	free(text);

	free(lineBreaks);

	return tempSurface;
}
