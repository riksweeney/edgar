#include "../headers.h"

#include "sprites.h"
#include "graphics.h"
#include "../system/pak.h"

static Sprite sprite[MAX_SPRITES];

static int loadSprite(char *);

static int spriteID = 0;

void loadSpritesFromFile(char *name, int *index)
{
	char *line;
	unsigned char *buffer;
	int i;
	
	buffer = loadFileFromPak(name);

	i = 0;
	
	line = strtok((char *)buffer, "\n");

	do
	{
		if (line[0] == '#' || line[0] == '\n')
		{
			line = strtok(NULL, "\n");
			
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
		
		line = strtok(NULL, "\n");
	}
	
	while (line != NULL);

	free(buffer);
}

static int loadSprite(char *name)
{
	int i, x, y, w, h, read;
	char filename[MAX_FILE_LENGTH];
	
	read = sscanf(name, "%s %d %d %d %d\n", filename, &x, &y, &w, &h);

	for (i=0;i<spriteID;i++)
	{
		if (strcmpignorecase(filename, sprite[i].name) == 0)
		{
			return i;
		}
	}

	sprite[spriteID].image = loadImage(filename);

	strncpy(sprite[spriteID].name, filename, MAX_FILE_LENGTH);
	
	if (read == 5)
	{
		printf("Setting bounding box information for %s\n", filename);
		
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
