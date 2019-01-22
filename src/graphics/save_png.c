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

#include "graphics.h"

#include <png.h>

/*
SDL PNG saving routine taken from pygame - Python Game Library
*/

static void writeData(char *name, png_bytep *rows, int w, int h, int colourtype, int bitdepth)
{
	png_structp pngPtr;
	png_infop infoPtr;
	FILE *fp;

	fp = fopen(name, "wb");

	if (fp == NULL)
	{
		printf("Could not save %s\n", name);

		exit(1);
	}

	pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (pngPtr == NULL)
	{
		printf("Failed to create PNG structure");

		exit(1);
	}

	infoPtr = png_create_info_struct(pngPtr);

	if (infoPtr == NULL)
	{
		printf("Failed to create PNG info");

		exit(1);
	}

	if (setjmp(png_jmpbuf(pngPtr)) != 0)
	{
		printf("PNG Jump point failed\n");

		exit(1);
	}

	png_init_io(pngPtr, fp);

	png_set_IHDR(pngPtr, infoPtr, w, h, bitdepth, colourtype, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(pngPtr, infoPtr);

	png_write_image(pngPtr, rows);

	png_write_end(pngPtr, NULL);

	png_destroy_write_struct(&pngPtr, &infoPtr);

	fclose(fp);
}

void savePNG(SDL_Surface *surface, char *name)
{
	unsigned char** ss_rows;
	int ss_size;
	int ss_w, ss_h;
	SDL_Surface *temp;
	SDL_Rect ss_rect;
	int i;

	ss_rows = 0;
	ss_size = 0;
	temp = NULL;

	ss_w = surface->w;
	ss_h = surface->h;

	temp = createSurface(surface->w, surface->h, FALSE);

	ss_rect.x = 0;
	ss_rect.y = 0;
	ss_rect.w = ss_w;
	ss_rect.h = ss_h;

	SDL_BlitSurface(surface, &ss_rect, temp, NULL);

	if (ss_size == 0)
	{
		ss_size = ss_h;
		ss_rows = malloc(sizeof (unsigned char*) * ss_size);

		if (ss_rows == NULL)
		{
			printf("Ran out of memory when creating PNG rows\n");

			exit(1);
		}
	}

	for (i = 0; i < ss_h; i++)
	{
		ss_rows[i] = ((unsigned char*)temp->pixels) + i * temp->pitch;
	}

	writeData(name, ss_rows, surface->w, surface->h, PNG_COLOR_TYPE_RGB_ALPHA, 8);

	free(ss_rows);

	SDL_FreeSurface(temp);

	temp = NULL;
}
