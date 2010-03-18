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

#include "headers.h"

#include "graphics/save_png.h"

static void cleanup(void);
static SDL_Surface *loadImage(char *);

static SDL_Surface *temp, *newSurface, *image, *screen;

int main(int argc, char *argv[])
{
	int x, y, i, OK;
	char name[20];
	SDL_Rect src;
	unsigned char r, g, b;
	int *pixels, xx, yy, pixel;
	int color;

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

	temp = SDL_CreateRGBSurface(SDL_HWSURFACE, TILE_SIZE, TILE_SIZE, image->format->BitsPerPixel, image->format->Rmask, image->format->Gmask, image->format->Bmask, 0);

	newSurface = SDL_DisplayFormat(temp);

	color = SDL_MapRGB(image->format, TRANS_R, TRANS_G, TRANS_B);

	for (y=0;y<image->h;y+=TILE_SIZE)
	{
		for (x=0;x<image->w;x+=TILE_SIZE)
		{
			SDL_FillRect(newSurface, NULL, color);

			src.x = x;
			src.y = y;
			src.w = image->w - x >= TILE_SIZE ? TILE_SIZE : image->w - x;
			src.h = image->h - y >= TILE_SIZE ? TILE_SIZE : image->h - y;

			SDL_BlitSurface(image, &src, newSurface, NULL);

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

			if (OK == 1)
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
