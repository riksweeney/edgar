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
#include "../system/pak.h"
#include "graphics.h"

extern Game game;

TTF_Font *loadFont(char *name, int size)
{
	TTF_Font *font = loadFontFromPak(name, size);

	if (font == NULL)
	{
		printf("Failed to open Font %s: %s\n", name, TTF_GetError());

		exit(1);
	}

	return font;
}

TTF_Font *loadCustomFont(char *name, int size)
{
	TTF_Font *font = TTF_OpenFont(name, size);

	if (font == NULL)
	{
		printf("Failed to open Font %s: %s\n", name, TTF_GetError());

		exit(1);
	}

	return font;
}

int getCharacterCodeForTestString()
{
	/* Will replace this with TTF_GlyphIsProvided when (if) SDL_TTF 2.0.10 becomes available in Ubuntu LTS */

	char *text[5];
	int i, length, ch, highest, j;

	text[0] = _("Press Action to interact");
	text[1] = _("Quickly turn left and right to shake off the slimes!");
	text[2] = _("Thanks, this is perfect!");
	text[3] = _("Yes");
	text[4] = _("No");

	highest = 0;

	for (j=0;j<5;j++)
	{
		length = strlen(text[j]);

		for (i=0;i<length;i++)
		{
			ch = ((unsigned char *)text[j])[i];

			if (ch >= 0xF0)
			{
				ch  = (int)(text[j][i]   & 0x07) << 18;
				ch |= (int)(text[j][++i] & 0x3F) << 12;
				ch |= (int)(text[j][++i] & 0x3F) << 6;
				ch |= (int)(text[j][++i] & 0x3F);
			}

			else if (ch >= 0xE0)
			{
				ch  = (int)(text[j][i]   & 0x0F) << 12;
				ch |= (int)(text[j][++i] & 0x3F) << 6;
				ch |= (int)(text[j][++i] & 0x3F);
			}

			else if (ch >= 0xC0)
			{
				ch  = (int)(text[j][i]   & 0x1F) << 6;
				ch |= (int)(text[j][++i] & 0x3F);
			}

			if (ch > highest)
			{
				highest = ch;
			}
		}
	}

	return highest;
}

void closeFont(TTF_Font *font)
{
	/* Close the font once we're done with it */

	if (font != NULL)
	{
		TTF_CloseFont(font);
	}
}

void drawString(char *text, int x, int y, TTF_Font *font, int centerX, int centerY, int r, int g, int b)
{
	SDL_Rect dest;
	SDL_Surface *surface;
	Texture *texture;
	SDL_Color foregroundcolour;

	foregroundcolour.r = r;
	foregroundcolour.g = g;
	foregroundcolour.b = b;

	/* Use SDL_TTF to generate a string image, this returns an SDL_Surface */

	surface = TTF_RenderText_Blended(font, text, foregroundcolour);

	if (surface == NULL)
	{
		printf("Couldn't create String %s: %s\n", text, SDL_GetError());

		return;
	}

	/* Blit the entire surface to the screen */

	dest.x = (centerX == 1 ? (SCREEN_WIDTH - surface->w) / 2 : x);
	dest.y = (centerY == 1 ? (SCREEN_HEIGHT - surface->h) / 2 : y);
	dest.w = surface->w;
	dest.h = surface->h;
	
	texture = convertSurfaceToTexture(surface, TRUE);

	SDL_RenderCopy(game.renderer, texture->texture, NULL, &dest);

	/* Free the generated string image */
	
	destroyTexture(texture);
}

SDL_Surface *generateTextSurface(char *text, TTF_Font *font, int fr, int fg, int fb, int br, int bg, int bb)
{
	SDL_Surface *textSurface, *backgroundSurface;
	SDL_Color foregroundcolour, backgroundcolour;

	foregroundcolour.r = fr;
	foregroundcolour.g = fg;
	foregroundcolour.b = fb;

	backgroundcolour.r = br;
	backgroundcolour.g = bg;
	backgroundcolour.b = bb;

	/* Use SDL_TTF to generate a string image, this returns an SDL_Texture */

	textSurface = TTF_RenderUTF8_Shaded(font, text, foregroundcolour, backgroundcolour);

	if (textSurface == NULL)
	{
		printf("Couldn't create String %s: %s\n", text, SDL_GetError());

		return NULL;
	}
	
	backgroundSurface = createSurface(textSurface->w, textSurface->h, FALSE);
	
	SDL_BlitSurface(textSurface, NULL, backgroundSurface, NULL);
	
	SDL_FreeSurface(textSurface);

	/* Return the generated string image */

	return backgroundSurface;
}

SDL_Surface *generateTransparentTextSurface(char *text, TTF_Font *font, int fr, int fg, int fb, int blend)
{
	SDL_Surface *surface;
	SDL_Color foregroundcolour;

	foregroundcolour.r = fr;
	foregroundcolour.g = fg;
	foregroundcolour.b = fb;

	if (blend == FALSE)
	{
		TTF_SetFontStyle(font, TTF_STYLE_BOLD);

		/* Use SDL_TTF to generate a string image, this returns an SDL_Texture */

		surface = TTF_RenderUTF8_Solid(font, text, foregroundcolour);

		TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
	}

	else
	{
		surface = TTF_RenderUTF8_Blended(font, text, foregroundcolour);
	}

	if (surface == NULL)
	{
		printf("Couldn't create String %s: %s\n", text, SDL_GetError());

		return NULL;
	}

	/* Return the generated string image */

	return surface;
}
