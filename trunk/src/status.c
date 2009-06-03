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

#include "graphics/font.h"

extern Game game;

/*
void doStatusPanel()
{
	message.thinkTime--;

	if (message.thinkTime <= 0)
	{
		message.thinkTime = 0;
	}
}

void drawStatusPanel()
{
	SDL_Rect dest;

	dest.x = 0;
	dest.y = SCREEN_HEIGHT - TILE_SIZE;
	dest.w = SCREEN_WIDTH;
	dest.h = TILE_SIZE;

	SDL_FillRect(game.screen, &dest, 0);

	if (message.thinkTime > 0)
	{
		drawString(message.text, 0, SCREEN_HEIGHT - TILE_SIZE, game.font, 1, 0, 255, 255, 255);
	}
}

void setStatusMessage(char *text)
{
	STRNCPY(message.text, text, sizeof(message.text));

	message.thinkTime = 120;
}
*/
