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

#include "headers.h"

#include "graphics/font.h"
#include "graphics/graphics.h"
#include "graphics/texture_cache.h"

extern Game game;

static Texture *panel;

void setStatusPanelMessage(char *fmt, ...)
{
	char text[MAX_MESSAGE_LENGTH];
	SDL_Surface *surface;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	if (strlen(text) > 0)
	{
		panel = getTextureFromCache(text);

		if (panel == NULL)
		{
			surface = generateTextSurface(text, game.font, 255, 255, 255, 0, 0, 0);

			panel = addBorder(surface, 255, 255, 255, 0, 0, 0);

			addTextureToCache(text, panel, FALSE);
		}
	}
}

void drawStatusPanel()
{
	if (panel != NULL)
	{
		drawImage(panel, (SCREEN_WIDTH - panel->w) / 2, SCREEN_HEIGHT - TILE_SIZE - 1, FALSE, 255);
	}
}
