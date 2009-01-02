#include "resources.h"

extern void loadPlayer(void);
extern void loadMapTiles(void);
extern void loadMapBackground(char *);
extern void loadMap(char *);
extern void loadBasicWeapons(void);
extern TTF_Font *loadFont(char *, int);
extern void freeSprites(void);
extern void freeMap(void);
extern void freeAnimations(void);
extern void freeSounds(void);

void loadResources()
{
	/* Load the player */

	loadPlayer();
	
	/* Load the weapons */
	
	loadBasicWeapons();
	
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
