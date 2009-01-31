#include "headers.h"

#include "animation.h"
#include "graphics.h"
#include "inventory.h"

static SDL_Surface *itemBox;

void initHud()
{
	itemBox = loadImage(INSTALL_PATH"gfx/hud/item_box.png");
}

void drawHud()
{
	drawSelectedInventoryItem((SCREEN_WIDTH - itemBox->w) / 2, 15, itemBox->w, itemBox->h);

	drawImage(itemBox, (SCREEN_WIDTH - itemBox->w) / 2, 15);
}

void freeHud()
{
	if (itemBox != NULL)
	{
		SDL_FreeSurface(itemBox);
	}
}
