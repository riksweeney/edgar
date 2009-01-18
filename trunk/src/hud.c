#include "hud.h"

extern SDL_Surface *loadImage(char *);
extern void drawImage(SDL_Surface *, int, int);
extern void drawSelectedInventoryItem(int, int, int, int);
extern Entity *getPlayer(void);
extern SDL_Surface *getSpriteImage(int);
extern void loadProperties(char *, Entity *);

void initHud()
{
	itemBox = loadImage(INSTALL_PATH"gfx/hud/item_box.png");
}

void drawHud()
{
	/*
	int i;
	Entity *player = getPlayer();
	*/
	drawSelectedInventoryItem((SCREEN_WIDTH - itemBox->w) / 2, 15, itemBox->w, itemBox->h);

	drawImage(itemBox, (SCREEN_WIDTH - itemBox->w) / 2, 15);
	/*
	for (i=1;i<=player->maxHealth;i++)
	{
		drawImage(getSpriteImage(i <= player->health ? HEART_FULL : HEART_EMPTY), 20 * (i - 1) + 5, 5);
	}
	*/
}

void freeHud()
{
	if (itemBox != NULL)
	{
		SDL_FreeSurface(itemBox);
	}
}
