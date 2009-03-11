#include "headers.h"

#include "animation.h"
#include "graphics.h"
#include "inventory.h"
#include "font.h"

extern Game game;
extern Entity player;

static SDL_Surface *itemBox, *heart, *emptyHeart;
static Message infoMessage;

void initHud()
{
	itemBox = loadImage(INSTALL_PATH"gfx/hud/item_box.png");
	
	heart = loadImage(INSTALL_PATH"gfx/hud/heart.png");
	
	emptyHeart = loadImage(INSTALL_PATH"gfx/hud/heart_empty.png");
}

void doHud()
{
	infoMessage.thinkTime--;

	if (infoMessage.thinkTime <= 0)
	{
		if (infoMessage.surface != NULL)
		{
			SDL_FreeSurface(infoMessage.surface);

			infoMessage.surface = NULL;

			infoMessage.text[0] = '\0';
		}
	}
}

void drawHud()
{
	int i, h, w;
	
	drawSelectedInventoryItem((SCREEN_WIDTH - itemBox->w) / 2, 15, itemBox->w, itemBox->h);

	drawImage(itemBox, (SCREEN_WIDTH - itemBox->w) / 2, 15, FALSE);

	if (infoMessage.surface != NULL)
	{
		drawBorder((SCREEN_WIDTH - infoMessage.surface->w) / 2, 400, infoMessage.surface->w, infoMessage.surface->h, 255, 255, 255);

		drawImage(infoMessage.surface, (SCREEN_WIDTH - infoMessage.surface->w) / 2, 400, FALSE);
	}
	
	w = h = 5;
	
	for (i=0;i<player.maxHealth;i++)
	{
		if (i != 0 && (i % 10) == 0)
		{
			h += heart->h;
			
			w = 5;
		}
		
		if (i < player.health)
		{
			drawImage(heart, w, h, FALSE);
		}
		
		else
		{
			drawImage(emptyHeart, w, h, FALSE);
		}
		
		w += heart->w + 5;
	}
}

void freeHud()
{
	if (itemBox != NULL)
	{
		SDL_FreeSurface(itemBox);

		itemBox = NULL;
	}
	
	if (heart != NULL)
	{
		SDL_FreeSurface(heart);

		heart = NULL;
	}
	
	if (emptyHeart != NULL)
	{
		SDL_FreeSurface(emptyHeart);

		emptyHeart = NULL;
	}

	if (infoMessage.surface != NULL)
	{
		SDL_FreeSurface(infoMessage.surface);

		infoMessage.surface = NULL;
	}
}

void setInfoBoxMessage(int thinkTime, char *fmt, ...)
{
	char text[MAX_MESSAGE_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	if (strcmpignorecase(text, infoMessage.text) != 0)
	{
		STRNCPY(infoMessage.text, text, sizeof(infoMessage.text));

		if (infoMessage.surface != NULL)
		{
			SDL_FreeSurface(infoMessage.surface);

			infoMessage.surface = NULL;
		}

		infoMessage.surface = generateTextSurface(infoMessage.text, game.font);
	}

	infoMessage.thinkTime = (thinkTime <= 0 ? 5 : thinkTime);
}
