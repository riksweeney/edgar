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

#include "../collisions.h"
#include "../entity.h"
#include "../map.h"
#include "../system/error.h"
#include "../system/pak.h"
#include "decoration.h"
#include "graphics.h"
#include "save_png.h"

extern Game game;
extern Entity *self;

static Uint32 getPixel(SDL_Surface *, int, int);
static void putPixel(int, int, Colour);
static void putPixelToSurface(SDL_Surface *, int, int, Uint32);
Texture *convertSurfaceToTexture(SDL_Surface *, int);
void destroyTexture(Texture *);

static char screenshotPath[MAX_PATH_LENGTH];
static int frame = 0;

Texture *loadImage(char *name)
{
	/* Load the image using SDL Image */

	SDL_Surface *image;
	Texture *texture;

	image = loadImageFromPak(name);

	texture = convertSurfaceToTexture(image, TRUE);

	/* Return the processed image */

	return texture;
}

SDL_Surface *loadImageAsSurface(char *name)
{
	/* Load the image using SDL Image */

	SDL_Surface *image;

	image = loadImageFromPak(name);

	/* Return the processed image */

	return image;
}

Texture *convertSurfaceToTexture(SDL_Surface *surface, int delete)
{
	SDL_Texture *texture;
	Texture *textureWrapper;

	textureWrapper = malloc(sizeof(Texture));

	if (textureWrapper == NULL)
	{
		showErrorAndExit("Failed to allocate %ld bytes to create texture", sizeof(Texture));
	}

	texture = SDL_CreateTextureFromSurface(game.renderer, surface);

	if (texture == NULL)
	{
		showErrorAndExit("Failed to convert image to texture");
	}

	textureWrapper->texture = texture;

	textureWrapper->w = surface->w;
	textureWrapper->h = surface->h;

	if (delete == TRUE)
	{
		SDL_FreeSurface(surface);
	}

	return textureWrapper;
}

Texture *createTexture(int width, int height, int r, int g, int b)
{
	Texture *texture;
	SDL_Surface *surface;

	surface = createSurface(width, height, FALSE);

	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, r, g, b));

	texture = convertSurfaceToTexture(surface, TRUE);

	return texture;
}

Texture *createWritableTexture(int width, int height)
{
	SDL_Texture *texture;
	Texture *textureWrapper;

	textureWrapper = malloc(sizeof(Texture));

	if (textureWrapper == NULL)
	{
		showErrorAndExit("Failed to allocate %ld bytes to create texture", sizeof(Texture));
	}

	texture = SDL_CreateTexture(game.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);

	if (texture == NULL)
	{
		showErrorAndExit("Failed to create writable texture");
	}

	textureWrapper->texture = texture;

	textureWrapper->w = width;
	textureWrapper->h = height;

	return textureWrapper;
}

void destroyTexture(Texture *image)
{
	if (image != NULL)
	{
		SDL_DestroyTexture(image->texture);

		free(image);
	}
}

void drawImage(Texture *image, int x, int y, int flip, int alpha)
{
	SDL_Rect dest;

	if (alpha == 0)
	{
		return;
	}

	/* Set the blitting rectangle to the size of the source image */

	dest.x = game.offsetX + x;
	dest.y = game.offsetY + y;
	dest.w = image->w;
	dest.h = image->h;

	if (alpha != 255 && alpha != -1)
	{
		SDL_SetTextureAlphaMod(image->texture, alpha);
	}

	/* Blit the entire image onto the screen at coordinates x and y */

	SDL_RenderCopyEx(game.renderer, image->texture, NULL, &dest, 0, NULL, flip == TRUE ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

	SDL_SetTextureAlphaMod(image->texture, 255);
}

void drawImageToMap(Texture *image, int x, int y, int flip, int alpha)
{
	drawImage(image, x - getMapStartX(), y - getMapStartY(), flip, alpha);
}

void drawClippedImage(Texture *image, int srcX, int srcY, int destX, int destY, int width, int height)
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

	SDL_RenderCopy(game.renderer, image->texture, &src, &dest);
}

void drawBoxToSurface(SDL_Surface *surface, int x, int y, int w, int h, int r, int g, int b)
{
	SDL_Rect rect;
	Uint32 colour;

	colour = SDL_MapRGB(surface->format, r, g, b);

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	SDL_FillRect(surface, &rect, colour);
}

void drawBox(int x, int y, int w, int h, int r, int g, int b, int a)
{
	SDL_Rect rect;

	SDL_SetRenderDrawColor(game.renderer, r, g, b, a);

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	SDL_RenderFillRect(game.renderer, &rect);
}

void drawBoxToMap(int x, int y, int w, int h, int r, int g, int b)
{
	SDL_Rect rect;

	rect.x = x - getMapStartX();
	rect.y = y - getMapStartY();
	rect.w = w;
	rect.h = h;

	if (collision(rect.x, rect.y, rect.w, rect.h, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) == TRUE)
	{
		SDL_SetRenderDrawColor(game.renderer, r, g, b, 255);

		SDL_RenderFillRect(game.renderer, &rect);
	}
}

void putPixelToMap(int x, int y, int r, int g, int b)
{
	int startX, startY;
	Colour colour;

	startX = getMapStartX();
	startY = getMapStartY();

	x -= startX;
	y -= startY;

	if (collision(x, y, 1, 1, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) == FALSE)
	{
		return;
	}

	colour.r = r;
	colour.g = g;
	colour.b = b;
	colour.a = 255;

	putPixel(x, y, colour);
}

void drawLine(int x1, int y1, int x2, int y2, int r, int g, int b)
{
	int lDelta, sDelta, cycle, lStep, sStep;
	int startX, startY;
	int clipX, clipY, clipW, clipH;
	SDL_Rect clipRect;
	Colour colour;

	colour.r = r;
	colour.g = g;
	colour.b = b;
	colour.a = 255;

	startX = getMapStartX();
	startY = getMapStartY();

	SDL_RenderGetClipRect(game.renderer, &clipRect);

	clipX = clipRect.x;
	clipY = clipRect.y;

	clipW = clipRect.x + clipRect.w;
	clipH = clipRect.y + clipRect.h;
	
	if (clipX == 0 && clipY == 0 && clipW == 0 && clipH == 0)
	{
		clipW = SCREEN_WIDTH;
		clipH = SCREEN_HEIGHT;
	}

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

	if (sDelta < lDelta)
	{
		cycle = lDelta >> 1;

		while (x1 != x2)
		{
			if (x1 >= clipX && x1 < clipW && y1 >= clipY && y1 < clipH)
			{
				putPixel(x1, y1, colour);
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
			putPixel(x1, y1, colour);
		}
	}

	cycle = sDelta >> 1;

	while (y1 != y2)
	{
		if (x1 >= clipX && x1 < clipW && y1 >= clipY && y1 < clipH)
		{
			putPixel(x1, y1, colour);
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
		putPixel(x1, y1, colour);
	}
}

void drawColouredLine(int x1, int y1, int x2, int y2, Colour colour1, Colour colour2, Colour colour3)
{
	int lDelta, sDelta, cycle, lStep, sStep;
	int startX, startY;
	int clipX, clipY, clipW, clipH;
	SDL_Rect clipRect;

	startX = getMapStartX();
	startY = getMapStartY();

	SDL_RenderGetClipRect(game.renderer, &clipRect);

	clipX = clipRect.x;
	clipY = clipRect.y;

	clipW = clipRect.x + clipRect.w;
	clipH = clipRect.y + clipRect.h;
	
	if (clipX == 0 && clipY == 0 && clipW == 0 && clipH == 0)
	{
		clipW = SCREEN_WIDTH;
		clipH = SCREEN_HEIGHT;
	}

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

	if (sDelta < lDelta)
	{
		cycle = lDelta >> 1;

		while (x1 != x2)
		{
			if (x1 >= clipX && x1 < clipW && y1 - 2 >= clipY && y1 + 2 < clipH)
			{
				putPixel(x1, y1 - 2, colour3);
				putPixel(x1, y1 - 1, colour2);
				putPixel(x1, y1, colour1);
				putPixel(x1, y1 + 1, colour2);
				putPixel(x1, y1 + 2, colour3);
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
			putPixel(x1, y1 - 2, colour3);
			putPixel(x1, y1 - 1, colour2);
			putPixel(x1, y1, colour1);
			putPixel(x1, y1 + 1, colour2);
			putPixel(x1, y1 + 2, colour3);
		}
	}

	cycle = sDelta >> 1;

	while (y1 != y2)
	{
		if (x1 >= clipX && x1 < clipW && y1 - 2 >= clipY && y1 + 2 < clipH)
		{
			putPixel(x1, y1 - 2, colour3);
			putPixel(x1, y1 - 1, colour2);
			putPixel(x1, y1, colour1);
			putPixel(x1, y1 + 1, colour2);
			putPixel(x1, y1 + 2, colour3);
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
		putPixel(x1, y1 - 2, colour3);
		putPixel(x1, y1 - 1, colour2);
		putPixel(x1, y1, colour1);
		putPixel(x1, y1 + 1, colour2);
		putPixel(x1, y1 + 2, colour3);
	}
}

void drawCircle(int x, int y, int radius, int r, int g, int b)
{
    int xx = radius - 1;
    int yy = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - radius * 2;

    SDL_SetRenderDrawColor(game.renderer, r, g, b, 255);

    while (xx >= yy)
    {
		SDL_RenderDrawLine(game.renderer, x - xx, y + yy, x + xx, y + yy);

		SDL_RenderDrawLine(game.renderer, x - yy, y - xx, x + yy, y - xx);

		SDL_RenderDrawLine(game.renderer, x - xx, y - yy, x + xx, y - yy);

		SDL_RenderDrawLine(game.renderer, x - yy, y + xx, x + yy, y + xx);

        if (err <= 0)
        {
            yy++;
            err += dy;
            dy += 2;
        }

        if (err > 0)
        {
            xx--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

void drawCircleFromSurface(int x, int y, int radius)
{
    int xx = radius - 1;
    int yy = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - radius * 2;
    SDL_Rect rect;

    SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);

    rect.x = 0;
    rect.y = 0;
    rect.w = SCREEN_WIDTH;
    rect.h = y - radius + 1;

    if (rect.h > 0)
    {
    	SDL_RenderFillRect(game.renderer, &rect);
    }

    rect.x = 0;
    rect.y = y + radius;
    rect.w = SCREEN_WIDTH;
    rect.h = SCREEN_HEIGHT - y - radius;

    if (rect.h > 0)
    {
    	SDL_RenderFillRect(game.renderer, &rect);
    }

    while (xx >= yy)
    {
		SDL_RenderDrawLine(game.renderer, 0, y + yy, x - xx, y + yy);

		SDL_RenderDrawLine(game.renderer, x + xx, y + yy, SCREEN_WIDTH, y + yy);

		SDL_RenderDrawLine(game.renderer, 0, y - xx, x - yy, y - xx);

		SDL_RenderDrawLine(game.renderer, x + yy, y - xx, SCREEN_WIDTH, y - xx);

		SDL_RenderDrawLine(game.renderer, 0, y - yy, x - xx, y - yy);

		SDL_RenderDrawLine(game.renderer, x + xx, y - yy, SCREEN_WIDTH, y - yy);

		SDL_RenderDrawLine(game.renderer, 0, y + xx, x - yy, y + xx);

		SDL_RenderDrawLine(game.renderer, x + yy, y + xx, SCREEN_WIDTH, y + xx);

        if (err <= 0)
        {
            yy++;
            err += dy;
            dy += 2;
        }

        if (err > 0)
        {
            xx--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

Texture *addBorder(SDL_Surface *surface, int r, int g, int b, int br, int bg, int bb)
{
	int colour;
	SDL_Rect rect;
	SDL_Surface *newSurface;
	Texture *texture;

	newSurface = createSurface(surface->w + BORDER_PADDING * 2, surface->h + BORDER_PADDING * 2, FALSE);

	colour = SDL_MapRGB(surface->format, r, g, b);

	SDL_FillRect(newSurface, NULL, SDL_MapRGB(newSurface->format, br, bg, bb));

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

	SDL_FillRect(newSurface, &rect, colour);

	/* Left */

	rect.x = 0;
	rect.y = 0;
	rect.w = 1;
	rect.h = newSurface->h;

	SDL_FillRect(newSurface, &rect, colour);

	/* Right */

	rect.x = newSurface->w - 1;
	rect.y = 0;
	rect.w = 1;
	rect.h = newSurface->h;

	SDL_FillRect(newSurface, &rect, colour);

	/* Bottom */

	rect.x = 0;
	rect.y = newSurface->h - 1;
	rect.w = newSurface->w;
	rect.h = 1;

	SDL_FillRect(newSurface, &rect, colour);

	SDL_FreeSurface(surface);

	texture = convertSurfaceToTexture(newSurface, TRUE);

	return texture;
}

Texture *copyScreen()
{
	SDL_Surface *tempSurface;
	Texture *texture;

	tempSurface = createSurface(SCREEN_WIDTH, SCREEN_HEIGHT, TRUE);

	SDL_SetRenderDrawColor(game.renderer, 255, 255, 255, 255);

	SDL_RenderReadPixels(game.renderer, NULL, SDL_PIXELFORMAT_ARGB8888, tempSurface->pixels, tempSurface->pitch);

	texture = convertSurfaceToTexture(tempSurface, TRUE);

	return texture;
}

void clearScreen(int r, int g, int b)
{
	SDL_SetRenderDrawColor(game.renderer, r, g, b, 255);

	SDL_RenderClear(game.renderer);
}

void drawHitBox(int startX, int startY, int w, int h)
{
	int x, y;
	Uint32 red, transparent;
	SDL_Rect dest;
	SDL_Surface *image;
	Texture *texture;

	transparent = 0;

	image = createSurface(w, h, FALSE);

	red = SDL_MapRGB(image->format, 255, 0, 0);

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
				putPixelToSurface(image, x, y, red);
			}

			else if (x == 0 || x == (image->w - 1))
			{
				putPixelToSurface(image, x, y, red);
			}

			else
			{
				putPixelToSurface(image, x, y, transparent);
			}
		}
	}

	if (SDL_MUSTLOCK(image))
	{
		SDL_UnlockSurface(image);
	}

	texture = convertSurfaceToTexture(image, TRUE);

	dest.x = startX;
	dest.y = startY;
	dest.w = image->w;
	dest.h = image->h;

	SDL_RenderCopy(game.renderer, texture->texture, NULL, &dest);

	destroyTexture(texture);
}

Texture *convertImageToWhite(SDL_Surface *image, int delete)
{
	unsigned char r, g, b, a;
	int x, y;
	Uint32 white = SDL_MapRGB(image->format, 255, 255, 255);
	Uint32 pixel;
	SDL_Surface *whiteImage;
	Texture *texture;

	whiteImage = createSurface(image->w, image->h, FALSE);

	if (SDL_MUSTLOCK(image))
	{
		SDL_LockSurface(image);
	}

	for (x=0;x<image->w;x++)
	{
		for (y=0;y<image->h;y++)
		{
			pixel = getPixel(image, x, y);

			SDL_GetRGBA(pixel, image->format, &r, &g, &b, &a);
			
			if (a == 255)
			{
				putPixelToSurface(whiteImage, x, y, white);
			}
		}
	}

	if (SDL_MUSTLOCK(image))
	{
		SDL_UnlockSurface(image);
	}
	
	texture = convertSurfaceToTexture(whiteImage, delete);

	return texture;
}

EntityList *createPixelsFromSprite(Sprite *sprite)
{
	unsigned char r, g, b, a;
	int x, y;
	Uint32 pixel;
	SDL_Surface *image;
	Entity *d;
	EntityList *list;

	image = loadImageFromPak(sprite->name);

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

	for (y=0;y<image->h;y++)
	{
		for (x=0;x<image->w;x++)
		{
			pixel = getPixel(image, x, y);

			SDL_GetRGBA(pixel, image->format, &r, &g, &b, &a);

			if (a != 0)
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

	SDL_FreeSurface(image);

	return list;
}

SDL_Surface *createSurface(int width, int height, int useDefaults)
{
    SDL_Surface *newSurface;
    Uint32 rmask, gmask, bmask, amask;

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
	#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
	#endif

	if (useDefaults == TRUE)
	{
		newSurface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	}
	
	else
	{
		newSurface = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);
	}

	if (newSurface == NULL)
	{
		showErrorAndExit("Failed to create a surface");
	}

	return newSurface;
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

static void putPixel(int x, int y, Colour colour)
{
	SDL_SetRenderDrawColor(game.renderer, colour.r, colour.g, colour.b, colour.a);

	SDL_RenderDrawPoint(game.renderer, x, y);
}

static void putPixelToSurface(SDL_Surface *surface, int x, int y, Uint32 pixel)
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
	SDL_Surface *tempSurface;

	tempSurface = createSurface(SCREEN_WIDTH, SCREEN_HEIGHT, TRUE);

	SDL_RenderReadPixels(game.renderer, NULL, SDL_PIXELFORMAT_ARGB8888, tempSurface->pixels, tempSurface->pitch);

	if (strlen(screenshotPath) != 0)
	{
		snprintf(filename, sizeof(filename), "%s/edgar%06d.png", screenshotPath, frame);

		frame++;

		savePNG(tempSurface, filename);
	}

	SDL_FreeSurface(tempSurface);
}

void takeSingleScreenshot(char *name)
{
	SDL_Surface *tempSurface;

	tempSurface = createSurface(SCREEN_WIDTH, SCREEN_HEIGHT, TRUE);

	SDL_RenderReadPixels(game.renderer, NULL, SDL_PIXELFORMAT_ARGB8888, tempSurface->pixels, tempSurface->pitch);

	savePNG(tempSurface, name);

	SDL_FreeSurface(tempSurface);
}
