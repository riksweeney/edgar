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

#include "graphics/save_png.h"

static void cleanup(void);
static SDL_Surface *loadImage(char *);
static int isDuplicate(SDL_Surface *, int, int);
static Uint32 getPixel(SDL_Surface *, int, int);

static SDL_Surface *temp, *newSurface, *image, *screen;

int main(int argc, char *argv[])
{
	int x, y, i, OK, startIndex;
	char name[20];
	SDL_Rect src, dest;
	unsigned char r, g, b;
	int *pixels, xx, yy, pixel;
	int colour;

	i = 0;

	if (argc != 3)
	{
		printf("Usage: %s <PNG File> <Start_Index>\n", argv[0]);

		exit(0);
	}

	atexit(cleanup);

	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_AUDIO) < 0)
	{
		printf("Could not initialize SDL: %s\n", SDL_GetError());

		exit(1);
	}

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);

	image = loadImage(argv[1]);

	i = atoi(argv[2]);

	startIndex = i;

	temp = SDL_CreateRGBSurface(SDL_HWSURFACE, TILE_SIZE, TILE_SIZE, image->format->BitsPerPixel, image->format->Rmask, image->format->Gmask, image->format->Bmask, 0);

	newSurface = SDL_DisplayFormat(temp);

	colour = SDL_MapRGB(image->format, TRANS_R, TRANS_G, TRANS_B);

	for (y=0;y<image->h;y+=TILE_SIZE)
	{
		for (x=0;x<image->w;x+=TILE_SIZE)
		{
			SDL_FillRect(newSurface, NULL, colour);

			src.x = x;
			src.y = y;
			src.w = image->w - x >= TILE_SIZE ? TILE_SIZE : image->w - x;
			src.h = image->h - y >= TILE_SIZE ? TILE_SIZE : image->h - y;

			dest.x = 0;
			dest.y = 0;
			dest.w = src.w;
			dest.h = src.h;

			SDL_BlitSurface(image, &src, newSurface, &dest);

			sprintf(name, "%d.png", i);

			OK = 0;

			for (yy=0;yy<newSurface->h;yy++)
			{
				for (xx=0;xx<newSurface->w;xx++)
				{
					pixels = (int *)newSurface->pixels;

					pixel = pixels[(yy * newSurface->w) + xx];

					SDL_GetRGB(pixel, newSurface->format, &r, &g, &b);

					if (r != TRANS_R && g != TRANS_G && b != TRANS_B)
					{
						OK = 1;

						break;
					}
				}
			}

			if (OK == 1 && isDuplicate(newSurface, startIndex, i) == FALSE)
			{
				printf("Saving %s\n", name);

				savePNG(newSurface, name);

				i++;
			}

		}
	}

	exit(0);
}

static SDL_Surface *loadImage(char *name)
{
	/* Load the image using SDL Image */

	SDL_Surface *temp;
	SDL_Surface *image;

	temp = IMG_Load(name);

	if (temp == NULL)
	{
		printf("Failed to load image %s", name);

		exit(1);
	}

	/* Make the background transparent */

	SDL_SetColorKey(temp, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(temp->format, TRANS_R, TRANS_G, TRANS_B));

	/* Convert the image to the screen's native format */

	image = SDL_DisplayFormat(temp);

	SDL_FreeSurface(temp);

	if (image == NULL)
	{
		printf("Failed to convert image %s to native format", name);

		exit(1);
	}

	/* Return the processed image */

	return image;
}

static int isDuplicate(SDL_Surface *surface, int startIndex, int currentIndex)
{
	char name[10];
	int i, x, y, duplicate;
	SDL_Surface *temp;

	duplicate = FALSE;

	for (i=startIndex;i<currentIndex;i++)
	{
		sprintf(name, "%d.png", i);

		printf("Checking if %d.png is a duplicate of %d.png\n", currentIndex, startIndex);

		temp = loadImage(name);

		duplicate = TRUE;

		for (y=0;y<temp->h;y++)
		{
			for (x=0;x<temp->w;x++)
			{
				if (getPixel(temp, x, y) != getPixel(surface, x, y))
				{
					duplicate = FALSE;
				}
			}
		}

		SDL_FreeSurface(temp);

		if (duplicate == TRUE)
		{
			break;
		}
	}

	if (duplicate == TRUE)
	{
		printf("Image is a duplicate\n");
	}

	return duplicate;
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

static void cleanup()
{
	if (temp != NULL)
	{
		SDL_FreeSurface(temp);
	}

	if (newSurface != NULL)
	{
		SDL_FreeSurface(newSurface);
	}

	if (image != NULL)
	{
		SDL_FreeSurface(image);
	}

	SDL_Quit();
}
