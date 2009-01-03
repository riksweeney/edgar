#include "resources.h"

extern void loadMapTiles(void);
extern void loadMapBackground(char *);
extern void loadMap(char *);
extern void loadBasicWeapons(void);
extern TTF_Font *loadFont(char *, int);
extern void freeSprites(void);
extern void freeMap(void);
extern void freeAnimations(void);
extern void freeSounds(void);
extern void addBat(int, int);
extern void addApple(int, int);
extern void addWoodenCrate(int, int);
extern void addMetalCrate(int, int);
extern void addBat(int, int);
extern void loadPlayer(int, int);
extern void addKeyItem(char *, int, int);

void loadResources()
{
	/* Load the map */
	
	loadMap("data/maps/map01.dat");
	
	/* Load the font */
	
	game.font = loadFont("font/blackWolf.ttf", 16);
}

void freeResources()
{
	/* Free the animations */
	
	freeAnimations();
	
	/* Free the sounds */
	
	freeSounds();
	
	/* Free the map data */
	
	freeMap();
	
	/* Free the sprites */
	
	freeSprites();
}

void loadResource(char *name, int x, int y)
{
	int i;
	
	printf("Loading %s\n", name);
	
	for (i=0;i<strlen(name);i++)
	{
		name[i] = tolower(name[i]);
	}
	
	if (strcmp("apple", name) == 0)
	{
		addApple(x, y);
	}
	
	else if (strcmp("wooden_crate", name) == 0)
	{
		addWoodenCrate(x, y);
	}
	
	else if (strcmp("metal_crate", name) == 0)
	{
		addMetalCrate(x, y);
	}
	
	else if (strcmp("player_start", name) == 0)
	{
		loadPlayer(x, y);
	}
	
	else if (strcmp("bat", name) == 0)
	{
		addBat(x, y);
	}
	
	else if (strcmp("pickaxe", name) == 0)
	{
		addKeyItem(name, x, y);
	}
	
	else if (strcmp("small_wooden_shield", name) == 0)
	{
		addKeyItem(name, x, y);
	}
	
	else if (strcmp("basic_sword", name) == 0)
	{
		addKeyItem(name, x, y);
	}
	
	else
	{
		printf("****Unknown entity %s****\n", name);
	}
}
