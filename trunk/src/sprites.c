#include "headers.h"

static SDL_Surface *sprite[MAX_SPRITES];

static void loadSprite(char *);

static int spriteID = 0;

void loadSpritesFromFile(char *name, int *index)
{
	char line[MAX_LINE_LENGTH];
	int i;
	FILE *fp = fopen(name, "rb");

	printf("Loading sprites from %s\n", name);

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
	sprite[spriteID] = loadImage(name);
}

void freeSprites()
{
	int i;

	for (i=0;i<MAX_SPRITES;i++)
	{
		if (sprite[i] != NULL)
		{
			SDL_FreeSurface(sprite[i]);
		}
	}

	spriteID = 0;
}

SDL_Surface *getSpriteImage(int index)
{
	return sprite[index];
}
