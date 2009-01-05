#include "hud.h"

extern SDL_Surface *loadImage(char *);
extern void drawImage(SDL_Surface *, int, int);
extern void drawSelectedInventoryItem(int, int, int, int);
extern Entity *getPlayer(void);

void initHud()
{
	itemBox = loadImage(INSTALL_PATH"gfx/hud/item_box.png");
	
	heart[0] = loadImage(INSTALL_PATH"gfx/hud/heart_full.png");
	heart[1] = loadImage(INSTALL_PATH"gfx/hud/heart_empty.png");
}

void drawHud()
{
	int i;
	Entity *player = getPlayer();
	
	drawSelectedInventoryItem((SCREEN_WIDTH - itemBox->w) / 2, 15, itemBox->w, itemBox->h);
	
	drawImage(itemBox, (SCREEN_WIDTH - itemBox->w) / 2, 15);
	
	for (i=1;i<=player->maxHealth;i++)
	{
		if (i <= player->health)
		{
			drawImage(heart[0], 30 + 30 * i, 15);
		}
		
		else
		{
			drawImage(heart[1], 30 + 30 * i, 15);
		}
	}
}

void freeHud()
{
	if (itemBox != NULL)
	{
		SDL_FreeSurface(itemBox);
	}
	
	if (heart[0] != NULL)
	{
		SDL_FreeSurface(heart[0]);
	}
	
	if (heart[1] != NULL)
	{
		SDL_FreeSurface(heart[1]);
	}
}
