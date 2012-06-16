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

#include "../collisions.h"
#include "../entity.h"
#include "../map.h"
#include "../system/error.h"
#include "../system/pak.h"
#include "decoration.h"
#include "graphics.h"

extern Game game;
extern Entity *self;

static void drawImageWhite(SDL_Surface *, int, int);
static Uint32 getPixel(SDL_Surface *, int, int);
static void putPixel(SDL_Surface *, int, int, Uint32);

static char screenshotPath[MAX_PATH_LENGTH];
static int frame = 0;

SDL_Surface *loadImage(char *name)
{
	/* Load the image using SDL Image */

	SDL_Surface *temp;
	SDL_Surface *image;

	temp = loadImageFromPak(name);

	if (temp == NULL)
	{
		showErrorAndExit("Failed to load image %s", name);
	}

	/* Make the background transparent */

	SDL_SetColorKey(temp, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(temp->format, TRANS_R, TRANS_G, TRANS_B));

	/* Convert the image to the screen's native format */

	image = SDL_DisplayFormat(temp);

	SDL_FreeSurface(temp);

	if (image == NULL)
	{
		showErrorAndExit("Failed to convert image %s to native format", name);
	}

	/* Return the processed image */

	return image;
}

void drawImage(SDL_Surface *image, int x, int y, int white, int alpha)
{
	SDL_Rect dest;

	if (alpha == 0)
	{
		return;
	}

	if (white == TRUE)
	{
		drawImageWhite(image, x, y);

		return;
	}

	/* Set the blitting rectangle to the size of the source image */

	dest.x = game.offsetX + x;
	dest.y = game.offsetY + y;
	dest.w = image->w;
	dest.h = image->h;

	if (alpha != 255 && alpha != -1)
	{
		SDL_SetAlpha(image, SDL_SRCALPHA|SDL_RLEACCEL, alpha);
	}

	/* Blit the entire image onto the screen at coordinates x and y */

	SDL_BlitSurface(image, NULL, game.screen, &dest);

	SDL_SetAlpha(image, SDL_SRCALPHA|SDL_RLEACCEL, 255);
}

void drawClippedImage(SDL_Surface *image, int srcX, int srcY, int destX, int destY, int width, int height)
{
	SDL_Rect src, dest;

	src.x = srcX;
	src.y = srcY;
	src.w = width;
	src.h = height;

	dest.x = game.offsetX + destX;
	dest.y = game.offsetY + destY;
	dest.w = width;
	dest.h = height;

	SDL_BlitSurface(image, &src, game.screen, &dest);
}

SDL_Surface *copyImage(SDL_Surface *image, int x, int y, int w, int h)
{
	int x1, y1, x2, y2;
	Uint32 pixel;
	SDL_Surface *flipped;

	flipped = createSurface(w, h);

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	x2 = y2 = 0;

	for (y1=y;y1<y+h;y1++)
	{
		for (x1=x;x1<x+w;x1++)
		{
			pixel = getPixel(image, x1, y1);

			putPixel(flipped, x2, y2, pixel);

			x2++;
		}

		x2 = 0;

		y2++;
	}

	if (SDL_MUSTLOCK(image))
	{
		SDL_UnlockSurface(image);
	}

	if (image->flags & SDL_SRCCOLORKEY)
	{
		SDL_SetColorKey(flipped, SDL_RLEACCEL|SDL_SRCCOLORKEY, image->format->colorkey);
	}

	return flipped;
}

SDL_Surface *flipImage(SDL_Surface *image)
{
	int x, y;
	Uint32 pixel;
	SDL_Surface *flipped;

	flipped = createSurface(image->w, image->h);

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	for (y=0;y<image->h;y++)
	{
		for (x=0;x<image->w;x++)
		{
			pixel = getPixel(image, x, y);

			putPixel(flipped, image->w - 1 - x, y, pixel);
		}
	}

	if (SDL_MUSTLOCK(image))
	{
		SDL_UnlockSurface(image);
	}

	if (image->flags & SDL_SRCCOLORKEY)
	{
		SDL_SetColorKey(flipped, SDL_RLEACCEL|SDL_SRCCOLORKEY, image->format->colorkey);
	}

	return flipped;
}

void drawBox(SDL_Surface *surface, int x, int y, int w, int h, int r, int g, int b)
{
	int color = SDL_MapRGB(surface->format, r, g, b);
	SDL_Rect rect;

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	SDL_FillRect(surface, &rect, color);
}

void drawBoxToMap(int x, int y, int w, int h, int r, int g, int b)
{
	int color;
	SDL_Rect rect;

	rect.x = x - getMapStartX();
	rect.y = y - getMapStartY();
	rect.w = w;
	rect.h = h;

	if (collision(rect.x, rect.y, rect.w, rect.h, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) == TRUE)
	{
		color = SDL_MapRGB(game.screen->format, r, g, b);

		SDL_FillRect(game.screen, &rect, color);
	}
}

void putPixelToMap(int x, int y, int r, int g, int b)
{
	Uint32 color = SDL_MapRGB(game.screen->format, r, g, b);
	int startX, startY;

	startX = getMapStartX();
	startY = getMapStartY();

	x -= startX;
	y -= startY;

	if (collision(x, y, 1, 1, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) == FALSE)
	{
		return;
	}

	if (SDL_MUSTLOCK(game.screen))
	{
		SDL_LockSurface(game.screen);
	}

	putPixel(game.screen, x, y, color);

	if (SDL_MUSTLOCK(game.screen))
	{
		SDL_UnlockSurface(game.screen);
	}
}

void drawLine(int x1, int y1, int x2, int y2, int r, int g, int b)
{
	Uint32 color = SDL_MapRGB(game.screen->format, r, g, b);
	int lDelta, sDelta, cycle, lStep, sStep;
	int startX, startY;
	int clipX, clipY, clipW, clipH;
	SDL_Rect clipRect;

	startX = getMapStartX();
	startY = getMapStartY();

	SDL_GetClipRect(game.screen, &clipRect);

	clipX = clipRect.x;
	clipY = clipRect.y;

	clipW = clipRect.x + clipRect.w;
	clipH = clipRect.y + clipRect.h;

	x1 -= startX;
	y1 -= startY;

	x2 -= startX;
	y2 -= startY;

	if (collision(x1, y1, x2 - x1, y2 - y1, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) == FALSE)
	{
		return;
	}

	lDelta = x2 - x1;
	sDelta = y2 - y1;

	lStep = SIGN(lDelta);
	lDelta = abs(lDelta);

	sStep = SIGN(sDelta);
	sDelta = abs(sDelta);

	if (SDL_MUSTLOCK(game.screen))
	{
		SDL_LockSurface(game.screen);
	}

	if (sDelta < lDelta)
	{
		cycle = lDelta >> 1;

		while (x1 != x2)
		{
			if (x1 >= clipX && x1 < clipW && y1 >= clipY && y1 < clipH)
			{
				putPixel(game.screen, x1, y1, color);
			}

			cycle += sDelta;

			if (cycle > lDelta)
			{
				cycle -= lDelta;

				y1 += sStep;
			}

			x1 += lStep;
		}

		if (x1 >= clipX && x1 < clipW && y1 >= clipY && y1 < clipH)
		{
			putPixel(game.screen, x1, y1, color);
		}
	}

	cycle = sDelta >> 1;

	while (y1 != y2)
	{
		if (x1 >= clipX && x1 < clipW && y1 >= clipY && y1 < clipH)
		{
			putPixel(game.screen, x1, y1, color);
		}

		cycle += lDelta;

		if (cycle > sDelta)
		{
			cycle -= sDelta;

			x1 += lStep;
		}

		y1 += sStep;
	}

	if (x1 >= clipX && x1 < clipW && y1 >= clipY && y1 < clipH)
	{
		putPixel(game.screen, x1, y1, color);
	}

	if (SDL_MUSTLOCK(game.screen))
	{
		SDL_UnlockSurface(game.screen);
	}
}

void drawColouredLine(int x1, int y1, int x2, int y2, Uint32 color1, Uint32 color2, Uint32 color3)
{
	int lDelta, sDelta, cycle, lStep, sStep;
	int startX, startY;
	int clipX, clipY, clipW, clipH;
	SDL_Rect clipRect;

	startX = getMapStartX();
	startY = getMapStartY();

	SDL_GetClipRect(game.screen, &clipRect);

	clipX = clipRect.x;
	clipY = clipRect.y;

	clipW = clipRect.x + clipRect.w;
	clipH = clipRect.y + clipRect.h;

	x1 -= startX;
	y1 -= startY;

	x2 -= startX;
	y2 -= startY;

	if (collision(x1, y1, x2 - x1, y2 - y1, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) == FALSE)
	{
		return;
	}

	lDelta = x2 - x1;
	sDelta = y2 - y1;

	lStep = SIGN(lDelta);
	lDelta = abs(lDelta);

	sStep = SIGN(sDelta);
	sDelta = abs(sDelta);

	if (SDL_MUSTLOCK(game.screen))
	{
		SDL_LockSurface(game.screen);
	}

	if (sDelta < lDelta)
	{
		cycle = lDelta >> 1;

		while (x1 != x2)
		{
			if (x1 >= clipX && x1 < clipW && y1 - 2 >= clipY && y1 + 2 < clipH)
			{
				putPixel(game.screen, x1, y1 - 2, color3);
				putPixel(game.screen, x1, y1 - 1, color2);
				putPixel(game.screen, x1, y1, color1);
				putPixel(game.screen, x1, y1 + 1, color2);
				putPixel(game.screen, x1, y1 + 2, color3);
			}

			cycle += sDelta;

			if (cycle > lDelta)
			{
				cycle -= lDelta;

				y1 += sStep;
			}

			x1 += lStep;
		}

		if (x1 >= clipX && x1 < clipW && y1 - 2 >= clipY && y1 + 2 < clipH)
		{
			putPixel(game.screen, x1, y1 - 2, color3);
			putPixel(game.screen, x1, y1 - 1, color2);
			putPixel(game.screen, x1, y1, color1);
			putPixel(game.screen, x1, y1 + 1, color2);
			putPixel(game.screen, x1, y1 + 2, color3);
		}
	}

	cycle = sDelta >> 1;

	while (y1 != y2)
	{
		if (x1 >= clipX && x1 < clipW && y1 - 2 >= clipY && y1 + 2 < clipH)
		{
			putPixel(game.screen, x1, y1 - 2, color3);
			putPixel(game.screen, x1, y1 - 1, color2);
			putPixel(game.screen, x1, y1, color1);
			putPixel(game.screen, x1, y1 + 1, color2);
			putPixel(game.screen, x1, y1 + 2, color3);
		}

		cycle += lDelta;

		if (cycle > sDelta)
		{
			cycle -= sDelta;

			x1 += lStep;
		}

		y1 += sStep;
	}

	if (x1 >= clipX && x1 < clipW && y1 - 2 >= clipY && y1 + 2 < clipH)
	{
		putPixel(game.screen, x1, y1 - 2, color3);
		putPixel(game.screen, x1, y1 - 1, color2);
		putPixel(game.screen, x1, y1, color1);
		putPixel(game.screen, x1, y1 + 1, color2);
		putPixel(game.screen, x1, y1 + 2, color3);
	}

	if (SDL_MUSTLOCK(game.screen))
	{
		SDL_UnlockSurface(game.screen);
	}
}

void drawCircle(int x, int y, int radius, int r, int g, int b)
{
	int y1, y2, xr;
	int color = SDL_MapRGB(game.screen->format, r, g, b);
	SDL_Rect rect;

	for (y1=-radius,y2=radius;y1;++y1,--y2)
	{
		xr = (int)(sqrt(radius * radius - y1 * y1) + 0.5);

		rect.x = x - xr;
		rect.y = y + y1;
		rect.w = 2 * xr;
		rect.h = 1;

		SDL_FillRect(game.screen, &rect, color);

		rect.y = y + y2;
		rect.h = 1;

		SDL_FillRect(game.screen, &rect, color);
	}

	rect.x = x - radius;
	rect.y = y;
	rect.w = 2 * radius;
	rect.h = 1;

	SDL_FillRect(game.screen, &rect, color);
}

void drawCircleFromSurface(int x, int y, int radius)
{
	int y1, y2, xr;
	SDL_Rect src, dest;

	game.tempSurface = createSurface(game.screen->w, game.screen->h);

	SDL_BlitSurface(game.screen, NULL, game.tempSurface, NULL);

	SDL_FillRect(game.screen, NULL, 0);

	for (y1=-radius,y2=radius;y1;++y1,--y2)
	{
		xr = (int)(sqrt(radius * radius - y1 * y1) + 0.5);

		src.x = x - xr;
		src.y = y + y1;
		src.w = 2 * xr;
		src.h = 1;

		if (src.x < 0)
		{
			src.x = 0;
		}

		if (src.y < 0)
		{
			src.y = 0;
		}

		if (src.x + src.w > SCREEN_WIDTH)
		{
			src.w = SCREEN_WIDTH - src.x;
		}

		if (src.y + src.h > SCREEN_HEIGHT)
		{
			src.h = SCREEN_HEIGHT - src.y;
		}

		dest.x = x - xr;
		dest.y = y + y1;
		dest.w = 2 * xr;
		dest.h = 1;

		if (dest.x < 0)
		{
			dest.x = 0;
		}

		if (dest.y < 0)
		{
			dest.y = 0;
		}

		if (dest.x + dest.w > SCREEN_WIDTH)
		{
			dest.w = SCREEN_WIDTH - dest.x;
		}

		if (dest.y + dest.h > SCREEN_HEIGHT)
		{
			dest.h = SCREEN_HEIGHT - dest.y;
		}

		SDL_BlitSurface(game.tempSurface, &src, game.screen, &dest);

		src.y = y + y2;
		src.h = 1;

		if (src.x < 0)
		{
			src.x = 0;
		}

		if (src.y < 0)
		{
			src.y = 0;
		}

		if (src.x + src.w > SCREEN_WIDTH)
		{
			src.w = SCREEN_WIDTH - src.x;
		}

		if (src.y + src.h > SCREEN_HEIGHT)
		{
			src.h = SCREEN_HEIGHT - src.y;
		}

		dest.y = y + y2;
		dest.h = 1;

		if (dest.x < 0)
		{
			dest.x = 0;
		}

		if (dest.y < 0)
		{
			dest.y = 0;
		}

		if (dest.x + dest.w > SCREEN_WIDTH)
		{
			dest.w = SCREEN_WIDTH - dest.x;
		}

		if (dest.y + dest.h > SCREEN_HEIGHT)
		{
			dest.h = SCREEN_HEIGHT - dest.y;
		}

		SDL_BlitSurface(game.tempSurface, &src, game.screen, &dest);
	}

	src.x = x - radius;
	src.y = y;
	src.w = 2 * radius;
	src.h = 1;

	if (src.x < 0)
	{
		src.x = 0;
	}

	if (src.y < 0)
	{
		src.y = 0;
	}

	if (src.x + src.w > SCREEN_WIDTH)
	{
		src.w = SCREEN_WIDTH - src.x;
	}

	if (src.y + src.h > SCREEN_HEIGHT)
	{
		src.h = SCREEN_HEIGHT - src.y;
	}

	dest.x = x - radius;
	dest.y = y;
	dest.w = 2 * radius;
	dest.h = 1;

	if (dest.x < 0)
	{
		dest.x = 0;
	}

	if (dest.y < 0)
	{
		dest.y = 0;
	}

	if (dest.x + dest.w > SCREEN_WIDTH)
	{
		dest.w = SCREEN_WIDTH - dest.x;
	}

	if (dest.y + dest.h > SCREEN_HEIGHT)
	{
		dest.h = SCREEN_HEIGHT - dest.y;
	}

	SDL_BlitSurface(game.tempSurface, &src, game.screen, &dest);

	SDL_FreeSurface(game.tempSurface);

	game.tempSurface = NULL;
}

SDL_Surface *addBorder(SDL_Surface *surface, int r, int g, int b, int br, int bg, int bb)
{
	int color = SDL_MapRGB(game.screen->format, r, g, b);
	SDL_Rect rect;
	SDL_Surface *newSurface;

	newSurface = createSurface(surface->w + BORDER_PADDING * 2, surface->h + BORDER_PADDING * 2);

	SDL_FillRect(newSurface, NULL, SDL_MapRGB(game.screen->format, br, bg, bb));

	rect.x = BORDER_PADDING;
	rect.y = BORDER_PADDING;
	rect.w = surface->w;
	rect.h = surface->h;

	SDL_BlitSurface(surface, NULL, newSurface, &rect);

	/* Top */

	rect.x = 0;
	rect.y = 0;
	rect.w = newSurface->w;
	rect.h = 1;

	SDL_FillRect(newSurface, &rect, color);

	/* Left */

	rect.x = 0;
	rect.y = 0;
	rect.w = 1;
	rect.h = newSurface->h;

	SDL_FillRect(newSurface, &rect, color);

	/* Right */

	rect.x = newSurface->w - 1;
	rect.y = 0;
	rect.w = 1;
	rect.h = newSurface->h;

	SDL_FillRect(newSurface, &rect, color);

	/* Bottom */

	rect.x = 0;
	rect.y = newSurface->h - 1;
	rect.w = newSurface->w;
	rect.h = 1;

	SDL_FillRect(newSurface, &rect, color);

	SDL_SetColorKey(newSurface, SDL_RLEACCEL|SDL_SRCCOLORKEY, SDL_MapRGB(newSurface->format, TRANS_R, TRANS_G, TRANS_B));

	SDL_FreeSurface(surface);

	return newSurface;
}

void clearScreen(int r, int g, int b)
{
	int color = SDL_MapRGB(game.screen->format, r, g, b);

	SDL_FillRect(game.screen, NULL, color);
}

void drawHitBox(int startX, int startY, int w, int h)
{
	int x, y;
	Uint32 red, transparent;
	SDL_Rect dest;
	SDL_Surface *image;

	red = SDL_MapRGB(game.screen->format, 255, 0, 0);

	transparent = SDL_MapRGB(game.screen->format, TRANS_R, TRANS_G, TRANS_B);

	image = createSurface(w, h);

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	for (y=0;y<image->h;y++)
	{
		for (x=0;x<image->w;x++)
		{
			if (y == 0 || y == (image->h - 1))
			{
				putPixel(image, x, y, red);
			}

			else if (x == 0 || x == (image->w - 1))
			{
				putPixel(image, x, y, red);
			}

			else
			{
				putPixel(image, x, y, transparent);
			}
		}
	}

	if (SDL_MUSTLOCK(image))
	{
		SDL_UnlockSurface(image);
	}

	SDL_SetColorKey(image, SDL_RLEACCEL|SDL_SRCCOLORKEY, SDL_MapRGB(image->format, TRANS_R, TRANS_G, TRANS_B));

	dest.x = startX;
	dest.y = startY;
	dest.w = image->w;
	dest.h = image->h;

	SDL_BlitSurface(image, NULL, game.screen, &dest);

	SDL_FreeSurface(image);
}

static void drawImageWhite(SDL_Surface *image, int destX, int destY)
{
	unsigned char r, g, b, transR, transG, transB;
	int x, y;
	Uint32 pixel;
	Uint32 color = SDL_MapRGB(game.screen->format, 255, 255, 255);
	Uint32 transparent = SDL_MapRGB(game.screen->format, TRANS_R, TRANS_G, TRANS_B);
	SDL_Rect dest;
	SDL_Surface *flipped;

	flipped = createSurface(image->w, image->h);

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	SDL_GetRGB(transparent, game.screen->format, &transR, &transG, &transB);

	for (x=0;x<image->w;x++)
	{
		for (y=0;y<image->h;y++)
		{
			pixel = getPixel(image, x, y);

			SDL_GetRGB(pixel, game.screen->format, &r, &g, &b);

			putPixel(flipped, x, y, (r == transR && g == transG && b == transB) ? pixel : color);
		}
	}

	if (SDL_MUSTLOCK(image))
	{
		SDL_UnlockSurface(image);
	}

	if (image->flags & SDL_SRCCOLORKEY)
	{
		SDL_SetColorKey(flipped, SDL_RLEACCEL|SDL_SRCCOLORKEY, image->format->colorkey);
	}

	/* Set the blitting rectangle to the size of the src image */

	dest.x = game.offsetX + destX;
	dest.y = game.offsetY + destY;
	dest.w = flipped->w;
	dest.h = flipped->h;

	/* Blit the entire image onto the screen at coordinates x and y */

	SDL_BlitSurface(flipped, NULL, game.screen, &dest);

	SDL_FreeSurface(flipped);
}

EntityList *createPixelsFromSprite(Sprite *sprite)
{
	unsigned char r, g, b, transR, transG, transB;
	int x, y;
	Uint32 pixel;
	Uint32 transparent = SDL_MapRGB(game.screen->format, TRANS_R, TRANS_G, TRANS_B);
	SDL_Surface *image = sprite->image;
	Entity *d;
	EntityList *list;

	list = malloc(sizeof(EntityList));

	if (list == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Entity List", (int)sizeof(EntityList));
	}

	list->next = NULL;

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	SDL_GetRGB(transparent, game.screen->format, &transR, &transG, &transB);

	for (y=0;y<image->h;y++)
	{
		for (x=0;x<image->w;x++)
		{
			pixel = getPixel(image, x, y);

			SDL_GetRGB(pixel, game.screen->format, &r, &g, &b);

			if (r != transR && g != transG && b != transB)
			{
				d = addPixelDecoration(self->x + x, self->y + y);

				if (d != NULL)
				{
					d->health = r;

					d->maxHealth = g;

					d->mental = b;

					addEntityToList(list, d);
				}
			}
		}
	}

	if (SDL_MUSTLOCK(image))
	{
		SDL_UnlockSurface(image);
	}

	return list;
}

int isTransparent(SDL_Surface *image, int x, int y)
{
	Uint32 pixel;
	unsigned char r, g, b, transR, transG, transB;
	Uint32 transparent = SDL_MapRGB(game.screen->format, TRANS_R, TRANS_G, TRANS_B);

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	SDL_GetRGB(transparent, game.screen->format, &transR, &transG, &transB);

	pixel = getPixel(image, x, y);

	if (SDL_MUSTLOCK(image))
	{
		SDL_UnlockSurface(image);
	}

	SDL_GetRGB(pixel, game.screen->format, &r, &g, &b);

	return (r == transR && g == transG && b == transB);
}

SDL_Surface *createSurface(int width, int height)
{
	SDL_Surface *temp, *newSurface;

	temp = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, game.screen->format->BitsPerPixel, game.screen->format->Rmask, game.screen->format->Gmask, game.screen->format->Bmask, 0);

	newSurface = SDL_DisplayFormat(temp);

	SDL_FreeSurface(temp);

	return newSurface;
}

int getColour(int r, int g, int b)
{
	return SDL_MapRGB(game.screen->format, r, g, b);
}

static Uint32 getPixel(SDL_Surface *surface, int x, int y)
{
	int bpp = surface->format->BytesPerPixel;

	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp)
	{
		case 1:
		case 8:
			return *p;

		case 2:
		case 16:
			return *(Uint16 *)p;

		case 3:
		case 24:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			{
				return p[0] << 16 | p[1] << 8 | p[2];
			}

			else
			{
				return p[0] | p[1] << 8 | p[2] << 16;
			}

		case 4:
		case 32:
			return *(Uint32 *)p;

		default:
			return 0;
	}
}

static void putPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
	int bpp = surface->format->BytesPerPixel;

	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp)
	{
		case 1:
		case 8:
			*p = pixel;
		break;

		case 2:
		case 16:
			*(Uint16 *)p = pixel;
		break;

		case 3:
		case 24:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			{
				p[0] = (pixel >> 16) & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = pixel & 0xff;
			}

			else
			{
				p[0] = pixel & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = (pixel >> 16) & 0xff;
			}
		break;

		case 4:
		case 32:
			*(Uint32 *)p = pixel;
		break;
	}
}

void setScreenshotDir(char *name)
{
	STRNCPY(screenshotPath, name, sizeof(screenshotPath));

	printf("Set screenshot directory to %s\n", screenshotPath);
}

void takeScreenshot()
{
	char filename[MAX_PATH_LENGTH];

	if (strlen(screenshotPath) != 0)
	{
		snprintf(filename, sizeof(filename), "%s/edgar%06d.bmp", screenshotPath, frame);

		frame++;

		SDL_SaveBMP(game.screen, filename);
	}
}

void takeSingleScreenshot(char *name)
{
	SDL_SaveBMP(game.screen, name);
}
