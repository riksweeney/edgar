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

#include "../map.h"
#include "../collisions.h"
#include "../system/pak.h"
#include "graphics.h"
#include "../system/error.h"

extern Game game;

static void drawImageWhite(SDL_Surface *, int, int);

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
/*
void drawFlippedImage(SDL_Surface *image, int destX, int destY, int white, int alpha)
{
	unsigned char r, g, b;
	int *pixels, x, y, pixel, rx, ry;
	int color = SDL_MapRGB(game.screen->format, 255, 255, 255);
	SDL_Rect dest;
	SDL_Surface *flipped;

	flipped = createSurface(image->w, image->h);

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	for (x=0, rx=flipped->w-1;x<flipped->w;x++, rx--)
	{
		for (y=0, ry=flipped->h-1;y<flipped->h;y++, ry--)
		{
			pixels = (int *)image->pixels;

			pixel = pixels[(y * image->w) + x];

			pixels = (int *)flipped->pixels;

			if (white == TRUE)
			{
				SDL_GetRGB(pixel, game.screen->format, &r, &g, &b);

				pixels[(y * flipped->w) + rx] = (r == TRANS_R && g == TRANS_G && b == TRANS_B) ? pixel : color;
			}

			else
			{
				pixels[(y * flipped->w) + rx] = pixel;
			}
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
	
	if (alpha != 255 && white == FALSE)
	{
		SDL_SetAlpha(flipped, SDL_SRCALPHA|SDL_RLEACCEL, alpha);
	}

	Set the blitting rectangle to the size of the src image 

	dest.x = game.offsetX + destX;
	dest.y = game.offsetY + destY;
	dest.w = flipped->w;
	dest.h = flipped->h;

	Blit the entire image onto the screen at coordinates x and y 

	SDL_BlitSurface(flipped, NULL, game.screen, &dest);

	SDL_FreeSurface(flipped);
}
*/
SDL_Surface *flipImage(SDL_Surface *image)
{
	int *pixels, x, y, pixel, rx, ry;
	SDL_Surface *flipped;

	flipped = createSurface(image->w, image->h);

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	for (x=0, rx=flipped->w-1;x<flipped->w;x++, rx--)
	{
		for (y=0, ry=flipped->h-1;y<flipped->h;y++, ry--)
		{
			pixels = (int *)image->pixels;

			pixel = pixels[(y * image->w) + x];

			pixels = (int *)flipped->pixels;

			pixels[(y * flipped->w) + rx] = pixel;
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
	int color = SDL_MapRGB(game.screen->format, r, g, b);
	int startX, startY;
	int *pixels;

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
	
	pixels = (int *)game.screen->pixels;
	
	pixels[(y * game.screen->w) + x] = color;
	
	if (SDL_MUSTLOCK(game.screen))
	{
		SDL_UnlockSurface(game.screen);
	}
}

void drawLine(int x1, int y1, int x2, int y2, int r, int g, int b)
{
	int color = SDL_MapRGB(game.screen->format, r, g, b);
	int lDelta, sDelta, cycle, lStep, sStep;
	int startX, startY;
	int *pixels;

	startX = getMapStartX();
	startY = getMapStartY();

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

	pixels = (int *)game.screen->pixels;

	if (sDelta < lDelta)
	{
		cycle = lDelta >> 1;

		while (x1 != x2)
		{
			if (x1 >=0 && x1 < SCREEN_WIDTH && y1 >= 0 && y1 < SCREEN_HEIGHT)
			{
				pixels[(y1 * game.screen->w) + x1] = color;
			}

			cycle += sDelta;

			if (cycle > lDelta)
			{
				cycle -= lDelta;

				y1 += sStep;
			}

			x1 += lStep;
		}

		if (x1 >=0 && x1 < SCREEN_WIDTH && y1 >= 0 && y1 < SCREEN_HEIGHT)
		{
			pixels[(y1 * game.screen->w) + x1] = color;
		}
	}

	cycle = sDelta >> 1;

	while (y1 != y2)
	{
		if (x1 >=0 && x1 < SCREEN_WIDTH && y1 >= 0 && y1 < SCREEN_HEIGHT)
		{
			pixels[(y1 * game.screen->w) + x1] = color;
		}

		cycle += lDelta;

		if (cycle > sDelta)
		{
			cycle -= sDelta;

			x1 += lStep;
		}

		y1 += sStep;
	}

	if (x1 >=0 && x1 < SCREEN_WIDTH && y1 >= 0 && y1 < SCREEN_HEIGHT)
	{
		pixels[(y1 * game.screen->w) + x1] = color;
	}

	if (SDL_MUSTLOCK(game.screen))
	{
		SDL_UnlockSurface(game.screen);
	}
}

void drawCircle(int x, int y, int radius, int r, int g, int b)
{
	int y1, y2;
	int color = SDL_MapRGB(game.screen->format, r, g, b);
	SDL_Rect rect;

	for (y1=-radius,y2=radius;y1;++y1,--y2)
	{
		int xr = (int)(sqrt(radius * radius - y1 * y1) + 0.5);

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
	int y1, y2;
	SDL_Rect src, dest;

	game.tempSurface = createSurface(game.screen->w, game.screen->h);

	SDL_BlitSurface(game.screen, NULL, game.tempSurface, NULL);

	SDL_FillRect(game.screen, NULL, 0);

	for (y1=-radius,y2=radius;y1;++y1,--y2)
	{
		int xr = (int)(sqrt(radius * radius - y1 * y1) + 0.5);

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
}

SDL_Surface *addBorder(SDL_Surface *surface, int r, int g, int b, int br, int bg, int bb)
{
	int color = SDL_MapRGB(game.screen->format, r, g, b);
	SDL_Rect rect;
	SDL_Surface *newSurface;

	newSurface = createSurface(surface->w + 10, surface->h + 10);

	SDL_FillRect(newSurface, NULL, SDL_MapRGB(game.screen->format, br, bg, bb));

	rect.x = 5;
	rect.y = 5;
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
	int red, x, y, *pixels, transparent;
	SDL_Rect dest;
	SDL_Surface *image;
	
	pixels = NULL;

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
				pixels = (int *)image->pixels;

				pixels[(y * image->w) + x] = red;
			}

			else if (x == 0 || x == (image->w - 1))
			{
				pixels = (int *)image->pixels;

				pixels[(y * image->w) + x] = red;
			}

			else
			{
				pixels[(y * image->w) + x] = transparent;
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
	unsigned char r, g, b;
	int *pixels, x, y, pixel;
	int color = SDL_MapRGB(game.screen->format, 255, 255, 255);
	SDL_Rect dest;
	SDL_Surface *flipped;

	flipped = createSurface(image->w, image->h);

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	for (x=0;x<image->w;x++)
	{
		for (y=0;y<image->h;y++)
		{
			pixels = (int *)image->pixels;

			pixel = pixels[(y * image->w) + x];

			SDL_GetRGB(pixel, game.screen->format, &r, &g, &b);

			pixels = (int *)flipped->pixels;

			pixels[(y * flipped->w) + x] = (r == TRANS_R && g == TRANS_G && b == TRANS_B) ? pixel : color;
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

int isTransparent(SDL_Surface *image, int x, int y)
{
	int *pixels, pixel;
	unsigned char r, g, b;

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	pixels = (int *)image->pixels;

	if (SDL_MUSTLOCK(image))
	{
		SDL_UnlockSurface(image);
	}

	pixel = pixels[(y * image->w) + x];

	SDL_GetRGB(pixel, game.screen->format, &r, &g, &b);

	return (r == TRANS_R && g == TRANS_G && b == TRANS_B);
}

SDL_Surface *createSurface(int width, int height)
{
	SDL_Surface *temp, *newSurface;

	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, game.screen->format->BitsPerPixel, game.screen->format->Rmask, game.screen->format->Gmask, game.screen->format->Bmask, 0);

	newSurface = SDL_DisplayFormat(temp);

	SDL_FreeSurface(temp);

	return newSurface;
}
