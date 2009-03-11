#include "headers.h"

#include "sprites.h"
#include "graphics.h"

static Sprite sprite[MAX_SPRITES];

static void loadSprite(char *);

static int spriteID = 0;

void loadSpritesFromFile(char *name, int *index)
{
	char line[MAX_LINE_LENGTH];
	int i;
	FILE *fp = fopen(name, "rb");

	if (fp == NULL)
	{
		printf("Failed to open graphics file: %s\n", name);

		exit(1);
	}

	i = 0;

	while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
	{
		if (line[0] == '#' || line[0] == '\n')
		{
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

		loadSprite(line);

		index[i] = spriteID++;

		i++;
	}

	fclose(fp);
}

static void loadSprite(char *name)
{
	sprite[spriteID].image = loadImage(name);
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
	}

	spriteID = 0;
}

Sprite *getSprite(int index)
{
	return &sprite[index];
}
