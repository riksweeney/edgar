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
extern void loadPlayer(int, int);
extern void addKeyItem(char *, int, int);
extern void initHud(void);
extern void freeHud(void);

extern void addPermanentItem(char *, int, int);
extern void addLift(char *, int, int, int, int);
extern void addEnemy(char *, int, int);

void loadRequiredResources()
{
	/* Load the map */
	
	loadMap("data/maps/map01.dat");
	
	/* Load the hud */
	
	initHud();
	
	/* Load the font */
	
	game.font = loadFont("font/blackWolf.ttf", 16);
}

void freeRequiredResources()
{
	/* Free the animations */
	
	freeAnimations();
	
	/* Free the sounds */
	
	freeSounds();
	
	/* Free the map data */
	
	freeMap();
	
	/* Free the sprites */
	
	freeSprites();
	
	/* Free the hud */
	
	freeHud();
}

void loadResource(char *line)
{
	char type[20], name[20];
	int startX, startY, endX, endY;
	
	sscanf(line, "%s %s %d %d %d %d", type, name, &startX, &startY, &endX, &endY);
	
	if (strcmpignorecase(type, "ITEM") == 0 || strcmpignorecase(type, "HEALTH") == 0 ||
		strcmpignorecase(type, "SHIELD") == 0 || strcmpignorecase(type, "WEAPON") == 0)
	{
		addPermanentItem(name, startX, startY);
	}
	
	else if (strcmpignorecase(type, "player_start") == 0)
	{
		loadPlayer(startX, startY);
	}
	/*
	else if (strcmpignorecase(type, "KEY_ITEM") == 0)
	{
		addKeyItem(name, startX, startY);
	}
	
	else if (strcmpignorecase(type, "LIFT") == 0)
	{
		addLift(name, startX, startY, endX, endY);
	}
	*/
	else if (strcmpignorecase(type, "ENEMY") == 0)
	{
		addEnemy(name, startX, startY);
	}
	
	else
	{
		printf("Unknown Entity type %s\n", type);
	}
}
