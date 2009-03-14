#include "headers.h"

#include "graphics/animation.h"
#include "graphics/graphics.h"
#include "inventory.h"
#include "graphics/font.h"

extern Game game;
extern Entity player;

static Hud hud;

void initHud()
{
	hud.itemBox = loadImage(INSTALL_PATH"gfx/hud/item_box.png");

	hud.heart = loadImage(INSTALL_PATH"gfx/hud/heart.png");

	hud.emptyHeart = loadImage(INSTALL_PATH"gfx/hud/heart_empty.png");
}

void doHud()
{
	hud.thinkTime--;

	if (hud.thinkTime <= 0)
	{
		hud.thinkTime = 60;
	}

	hud.infoMessage.thinkTime--;

	if (hud.infoMessage.thinkTime <= 0)
	{
		if (hud.infoMessage.surface != NULL)
		{
			SDL_FreeSurface(hud.infoMessage.surface);

			hud.infoMessage.surface = NULL;

			hud.infoMessage.text[0] = '\0';
		}
	}
}

void drawHud()
{
	int i, h, w;

	drawSelectedInventoryItem((SCREEN_WIDTH - hud.itemBox->w) / 2, 15, hud.itemBox->w, hud.itemBox->h);

	drawImage(hud.itemBox, (SCREEN_WIDTH - hud.itemBox->w) / 2, 15, FALSE);

	if (hud.infoMessage.surface != NULL)
	{
		drawBorder((SCREEN_WIDTH - hud.infoMessage.surface->w) / 2, 400, hud.infoMessage.surface->w, hud.infoMessage.surface->h, 255, 255, 255);

		drawImage(hud.infoMessage.surface, (SCREEN_WIDTH - hud.infoMessage.surface->w) / 2, 400, FALSE);
	}

	w = h = 5;

	for (i=0;i<player.maxHealth;i++)
	{
		if (i != 0 && (i % 10) == 0)
		{
			h += hud.heart->h;

			w = 5;
		}

		if (i < player.health)
		{
			drawImage(hud.heart, w, h, (player.health <= 3 && hud.thinkTime <= 30));
		}

		else
		{
			drawImage(hud.emptyHeart, w, h, FALSE);
		}

		w += hud.heart->w + 5;
	}
}

void freeHud()
{
	if (hud.itemBox != NULL)
	{
		SDL_FreeSurface(hud.itemBox);

		hud.itemBox = NULL;
	}

	if (hud.heart != NULL)
	{
		SDL_FreeSurface(hud.heart);

		hud.heart = NULL;
	}

	if (hud.emptyHeart != NULL)
	{
		SDL_FreeSurface(hud.emptyHeart);

		hud.emptyHeart = NULL;
	}

	if (hud.infoMessage.surface != NULL)
	{
		SDL_FreeSurface(hud.infoMessage.surface);

		hud.infoMessage.surface = NULL;
	}
}

void setInfoBoxMessage(int thinkTime, char *fmt, ...)
{
	char text[MAX_MESSAGE_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	if (strcmpignorecase(text, hud.infoMessage.text) != 0)
	{
		STRNCPY(hud.infoMessage.text, text, sizeof(hud.infoMessage.text));

		if (hud.infoMessage.surface != NULL)
		{
			SDL_FreeSurface(hud.infoMessage.surface);

			hud.infoMessage.surface = NULL;
		}

		hud.infoMessage.surface = generateTextSurface(hud.infoMessage.text, game.font, 255, 255, 255, 0, 0, 0);
	}

	hud.infoMessage.thinkTime = (thinkTime <= 0 ? 5 : thinkTime);
}
