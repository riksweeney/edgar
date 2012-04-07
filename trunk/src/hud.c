/*
Copyright (C) 2009-2012 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include "headers.h"

#include "audio/audio.h"
#include "graphics/font.h"
#include "graphics/graphics.h"
#include "hud.h"
#include "inventory.h"
#include "medal.h"
#include "system/error.h"

extern Game game;
extern Entity player, *self, playerWeapon;

static Hud hud;
static Message messageHead;

static void addMessageToQueue(char *, int, int, int, int);
static void getNextMessageFromQueue(void);

void initHud()
{
	hud.itemBox = loadImage("gfx/hud/item_box.png");

	hud.heart = loadImage("gfx/hud/heart.png");

	hud.emptyHeart = loadImage("gfx/hud/heart_empty.png");

	hud.spotlight = loadImage("gfx/hud/spotlight.png");

	hud.medalSurface[0] = loadImage("gfx/hud/bronze_medal.png");

	hud.medalSurface[1] = loadImage("gfx/hud/silver_medal.png");

	hud.medalSurface[2] = loadImage("gfx/hud/gold_medal.png");

	hud.medalSurface[3] = loadImage("gfx/hud/ruby_medal.png");

	hud.disabledMedalSurface = loadImage("gfx/hud/disabled_medal.png");

	messageHead.next = NULL;

	hud.bossHealth = NULL;

	hud.medalTextSurface = NULL;
}

void doHud()
{
	hud.thinkTime--;

	if (hud.thinkTime <= 0)
	{
		hud.thinkTime = 60;
	}

	hud.medalThinkTime--;

	if (hud.medalThinkTime <= 0)
	{
		if (hud.medalTextSurface != NULL)
		{
			SDL_FreeSurface(hud.medalTextSurface);

			hud.medalTextSurface = NULL;

			medalProcessingFinished();
		}

		hud.medalThinkTime = 0;
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

		getNextMessageFromQueue();
	}

	if (hud.bossHealth != NULL)
	{
		if (hud.bossHealthIndex < *hud.bossHealth)
		{
			hud.bossHealthIndex += (hud.bossMaxHealth / 100);

			if (hud.bossHealthIndex > *hud.bossHealth)
			{
				hud.bossHealthIndex = *hud.bossHealth;
			}
		}

		else if (*hud.bossHealth < hud.bossHealthIndex)
		{
			hud.bossHealthIndex -= 3;

			if (hud.bossHealthIndex < *hud.bossHealth)
			{
				hud.bossHealthIndex = *hud.bossHealth;
			}
		}
	}
}

void drawHud()
{
	char quantity[4];
	int i, x, y, h, w, itemBoxMid, quant;
	float percentage, clipWidth;
	Entity *e;

	if (game.showHUD == TRUE)
	{
		itemBoxMid = (SCREEN_WIDTH - hud.itemBox->w) / 2;

		if (game.status == IN_INVENTORY)
		{
			drawBox(game.screen, itemBoxMid, 15, hud.itemBox->w, hud.itemBox->h, 0, 0, 0);
		}

		drawSelectedInventoryItem(itemBoxMid, 15, hud.itemBox->w, hud.itemBox->h);

		drawImage(hud.itemBox, itemBoxMid, 15, FALSE, 255);

		if (playerWeapon.inUse == TRUE)
		{
			x = FALSE;

			if (strcmpignorecase(playerWeapon.name, "weapon/bow") == 0)
			{
				e = getInventoryItemByObjectiveName(playerWeapon.requires);

				if (e != NULL)
				{
					x = TRUE;

					quant = e->health;
				}
			}

			else if (strcmpignorecase(playerWeapon.name, "weapon/lightning_sword") == 0)
			{
				x = TRUE;

				quant = playerWeapon.mental;
			}

			if (x == TRUE)
			{
				if (quant < 0)
				{
					quant = 0;
				}

				if (hud.quantity != quant)
				{
					if (hud.quantitySurface != NULL)
					{
						SDL_FreeSurface(hud.quantitySurface);
					}

					snprintf(quantity, 4, "%d", quant);

					hud.quantitySurface = generateTransparentTextSurface(quantity, game.font, 255, 255, 255, FALSE);
				}

				drawImage(hud.quantitySurface, (SCREEN_WIDTH - hud.quantitySurface->w) / 2, 15 + hud.itemBox->h + 5, FALSE, 255);
			}
		}

		else
		{
			hud.quantity = -1;
		}

		percentage = 0;

		if (hud.bossHealth != NULL)
		{
			x = SCREEN_WIDTH - 6;
			y = 5;

			x -= (hud.heart->w + 5) * 10;

			percentage = hud.bossHealthIndex * 100;

			percentage /= hud.bossMaxHealth;

			for (i=10;i<=100;i+=10)
			{
				if (i <= percentage)
				{
					drawImage(hud.heart, x, y, FALSE, 255);
				}

				else if (i - 10 < percentage)
				{
					clipWidth = (percentage - (i - 10)) / 10;

					w = hud.heart->w * clipWidth;

					drawClippedImage(hud.heart, 0, 0, x, y, w, hud.heart->h);
				}

				drawImage(hud.emptyHeart, x, y, FALSE, 255);

				x += hud.heart->w + 5;
			}
		}

		if (hud.infoMessage.surface != NULL)
		{
			drawImage(hud.infoMessage.surface, (SCREEN_WIDTH - hud.infoMessage.surface->w) / 2, SCREEN_HEIGHT - TILE_SIZE - 1, FALSE, 255);
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
				drawImage(hud.heart, w, h, (player.health <= 3 && hud.thinkTime <= 30), 255);
			}

			drawImage(hud.emptyHeart, w, h, FALSE, 255);

			w += hud.heart->w + 5;
		}
	}

	if (hud.medalTextSurface != NULL)
	{
		x = SCREEN_WIDTH - hud.medalTextSurface->w - 5;

		drawImage(hud.medalTextSurface, x, 5, FALSE, 255);
	}
}

void freeHud()
{
	int i;

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

	if (hud.spotlight != NULL)
	{
		SDL_FreeSurface(hud.spotlight);

		hud.spotlight = NULL;
	}

	if (hud.infoMessage.surface != NULL)
	{
		SDL_FreeSurface(hud.infoMessage.surface);

		hud.infoMessage.surface = NULL;
	}

	if (hud.medalTextSurface != NULL)
	{
		SDL_FreeSurface(hud.medalTextSurface);

		hud.medalTextSurface = NULL;
	}

	if (hud.quantitySurface != NULL)
	{
		SDL_FreeSurface(hud.quantitySurface);

		hud.quantitySurface = NULL;
	}

	for (i=0;i<4;i++)
	{
		if (hud.medalSurface[i] != NULL)
		{
			SDL_FreeSurface(hud.medalSurface[i]);

			hud.medalSurface[i] = NULL;
		}
	}
	
	if (hud.disabledMedalSurface != NULL)
	{
		SDL_FreeSurface(hud.disabledMedalSurface);

		hud.disabledMedalSurface = NULL;
	}

	freeMessageQueue();
}

void setInfoBoxMessage(int thinkTime, int r, int g, int b, char *fmt, ...)
{
	char text[MAX_MESSAGE_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	addMessageToQueue(text, thinkTime, r, g, b);
}

static void addMessageToQueue(char *text, int thinkTime, int r, int g, int b)
{
	Message *head, *msg;

	head = &messageHead;

	while (head->next != NULL)
	{
		if (strcmpignorecase(text, head->text) == 0)
		{
			return;
		}

		head = head->next;
	}

	msg = malloc(sizeof(Message));

	if (msg == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes for message queue", (int)sizeof(Message));
	}

	STRNCPY(msg->text, text, sizeof(messageHead.text));

	msg->thinkTime = thinkTime;

	msg->r = r;
	msg->g = g;
	msg->b = b;

	msg->next = NULL;

	head->next = msg;
}

static void getNextMessageFromQueue()
{
	Message *head = messageHead.next;

	if (head != NULL)
	{
		STRNCPY(hud.infoMessage.text, head->text, sizeof(hud.infoMessage.text));

		hud.infoMessage.r = head->r;
		hud.infoMessage.g = head->g;
		hud.infoMessage.b = head->b;

		if (hud.infoMessage.surface != NULL)
		{
			SDL_FreeSurface(hud.infoMessage.surface);

			hud.infoMessage.surface = NULL;
		}

		hud.infoMessage.surface = generateTextSurface(hud.infoMessage.text, game.font, hud.infoMessage.r, hud.infoMessage.g, hud.infoMessage.b, 0, 0, 0);

		hud.infoMessage.surface = addBorder(hud.infoMessage.surface, 255, 255, 255, 0, 0, 0);

		hud.infoMessage.thinkTime = (head->thinkTime <= 0 ? 5 : head->thinkTime);

		messageHead.next = head->next;

		free(head);
	}
}

void freeMessageQueue()
{
	Message *p, *q;

	for (p=messageHead.next;p!=NULL;p=q)
	{
		q = p->next;

		free(p);
	}

	messageHead.next = NULL;

	if (hud.infoMessage.surface != NULL)
	{
		SDL_FreeSurface(hud.infoMessage.surface);

		hud.infoMessage.surface = NULL;

		hud.infoMessage.text[0] = '\0';

		hud.infoMessage.thinkTime = 0;
	}
}

void initBossHealthBar()
{
	hud.bossHealth = &self->health;

	hud.bossMaxHealth = self->health;

	hud.bossHealthIndex = 0;
}

void freeBossHealthBar()
{
	hud.bossHealth = NULL;
}

void drawSpotlight(int x, int y)
{
	drawImage(hud.spotlight, x - game.offsetX, y - game.offsetY, FALSE, 255);
}

void showMedal(int medalType, char *message)
{
	SDL_Surface *textSurface, *medalSurface;
	SDL_Rect dest;

	if (hud.medalTextSurface != NULL)
	{
		return;
	}

	textSurface = generateTextSurface(message, game.font, 0, 220, 0, 0, 0, 0);

	medalSurface = createSurface(textSurface->w + hud.medalSurface[medalType]->w + 18, MAX(textSurface->h, hud.medalSurface[medalType]->h));

	dest.x = 5;
	dest.y = hud.medalSurface[medalType]->h / 2 - medalSurface->h / 2;
	dest.w = hud.medalSurface[medalType]->w;
	dest.h = hud.medalSurface[medalType]->h;

	SDL_BlitSurface(hud.medalSurface[medalType], NULL, medalSurface, &dest);

	dest.x = hud.medalSurface[medalType]->w + 13;
	dest.y = hud.medalSurface[medalType]->h / 2 - textSurface->h / 2;
	dest.w = textSurface->w;
	dest.h = textSurface->h;

	SDL_BlitSurface(textSurface, NULL, medalSurface, &dest);

	hud.medalTextSurface = addBorder(medalSurface, 255, 255, 255, 0, 0, 0);

	hud.medalThinkTime = 180;

	SDL_FreeSurface(textSurface);

	playSound("sound/common/trophy.ogg");

	return;
}

int spotlightSize()
{
	return hud.spotlight->w;
}

SDL_Surface *getMedalImage(int medalType, int obtained)
{
	return obtained == TRUE ? hud.medalSurface[medalType] : hud.disabledMedalSurface;
}
