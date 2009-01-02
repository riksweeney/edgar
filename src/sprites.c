#include "sprites.h"

extern SDL_Surface *loadImage(char *);

static void loadSpriteToBank(char *, int);

void loadSpritesFromFile(char *name)
{
	char imageName[255], line[MAX_LINE_LENGTH];
	int index;
	FILE *fp = fopen(name, "rb");
	
	printf("Loading sprites from %s\n", name);
	
	if (fp == NULL)
	{
		printf("Failed to open graphics file: %s\n", name);
		
		exit(1);
	}
	
	while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
	{
		if (line[0] == '#' || line[0] == '\n')
		{
			continue;
		}
		
		sscanf(line, "%s %d", imageName, &index);
		
		loadSpriteToBank(imageName, index);
	}
}

static void loadSpriteToBank(char *name, int index)
{
	if (sprite[index] != NULL)
	{
		printf("Attempting to replace existing sprite at %d\n", index);
		
		exit(1);
	}
	
	sprite[index] = loadImage(name);
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
