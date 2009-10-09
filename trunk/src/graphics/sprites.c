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

#include "sprites.h"
#include "graphics.h"
#include "../system/pak.h"

static Sprite sprite[MAX_SPRITES];

static int loadSprite(char *);

static int spriteID = 0;

void loadSpritesFromFile(char *name, int *index)
{
	char *line, *savePtr;
	unsigned char *buffer;
	int i;
	
	savePtr = NULL;

	buffer = loadFileFromPak(name);

	i = 0;

	line = strtok_r((char *)buffer, "\n", &savePtr);

	while (line != NULL)
	{
		if (line[0] == '#' || line[0] == '\n')
		{
			line = strtok_r(NULL, "\n", &savePtr);

			continue;
		}

		if (spriteID == MAX_SPRITES)
		{
			printf("Ran out of space for Sprites\n");

			abort();
		}

		if (line[strlen(line) - 1] == '\n')
		{
			line[strlen(line) - 1] = '\0';
		}

		if (line[strlen(line) - 1] == '\r')
		{
			line[strlen(line) - 1] = '\0';
		}

		index[i] = loadSprite(line);

		i++;

		line = strtok_r(NULL, "\n", &savePtr);
	}

	free(buffer);
}

static int loadSprite(char *name)
{
	int i, x, y, w, h, read, alpha;
	char filename[MAX_FILE_LENGTH];

	read = sscanf(name, "%s %d %d %d %d %d\n", filename, &x, &y, &w, &h, &alpha);

	for (i=0;i<spriteID;i++)
	{
		if (strcmpignorecase(filename, sprite[i].name) == 0)
		{
			return i;
		}
	}

	sprite[spriteID].image = loadImage(filename);

	STRNCPY(sprite[spriteID].name, filename, MAX_FILE_LENGTH);

	if (read == 5)
	{
		sprite[spriteID].box.x = x;
		sprite[spriteID].box.y = y;
		sprite[spriteID].box.w = w;
		sprite[spriteID].box.h = h;
	}

	else
	{
		sprite[spriteID].box.x = 0;
		sprite[spriteID].box.y = 0;
		sprite[spriteID].box.w = sprite[spriteID].image->w;
		sprite[spriteID].box.h = sprite[spriteID].image->h;
	}

	i = spriteID;

	spriteID++;
	
	sprite[spriteID].image = flipImage(sprite[spriteID - 1].image);

	STRNCPY(sprite[spriteID].name, filename, MAX_FILE_LENGTH);

	if (read == 5)
	{
		sprite[spriteID].box.x = x;
		sprite[spriteID].box.y = y;
		sprite[spriteID].box.w = w;
		sprite[spriteID].box.h = h;
	}

	else
	{
		sprite[spriteID].box.x = 0;
		sprite[spriteID].box.y = 0;
		sprite[spriteID].box.w = sprite[spriteID].image->w;
		sprite[spriteID].box.h = sprite[spriteID].image->h;
	}

	spriteID++;

	return i;
}

void freeSprites()
{
	int i;

	for (i=0;i<MAX_SPRITES;i++)
	{
		if (sprite[i].image != NULL)
		{
			SDL_FreeSurface(sprite[i].image);

			sprite[i].image = NULL;
		}

		sprite[i].name[0] = '\0';
	}

	spriteID = 0;
}

Sprite *getSprite(int index)
{
	return &sprite[index];
}
