#include "sprites.h"

extern SDL_Surface *loadImage(char *);

static void loadSprite(char *);

static int spriteID = 0;

void loadSpritesFromFile(char *name, int *index)
{
	char imageName[255], line[MAX_LINE_LENGTH];
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
		
		sscanf(line, "%s", imageName);
		
		loadSprite(imageName);
		
		index[i] = spriteID++;
		
		i++;
	}
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
}

SDL_Surface *getSpriteImage(int index)
{
	return sprite[index];
}
