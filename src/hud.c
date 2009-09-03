/*
Copyright (C) 2009 Parallel Realities

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "headers.h"

#include "graphics/animation.h"
#include "graphics/graphics.h"
#include "inventory.h"
#include "graphics/font.h"
#include "hud.h"

extern Game game;
extern Entity player, *self;

static Hud hud;
static Message messageHead;

static void addMessageToQueue(char *, int);
static void getNextMessageFromQueue(void);

void initHud()
{
	hud.itemBox = loadImage("gfx/hud/item_box.png");

	hud.heart = loadImage("gfx/hud/heart.png");

	hud.emptyHeart = loadImage("gfx/hud/heart_empty.png");

	messageHead.next = NULL;

	hud.bossHealth = NULL;
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

		getNextMessageFromQueue();
	}

	if (hud.bossHealth != NULL)
	{
		if (hud.bossHealthIndex < *hud.bossHealth)
		{
			hud.bossHealthIndex += (*hud.bossHealth / 100);

			if (hud.bossHealthIndex > *hud.bossHealth)
			{
				hud.bossHealthIndex = *hud.bossHealth;
			}
		}

		else if (*hud.bossHealth < hud.bossHealthIndex)
		{
			hud.bossHealthIndex--;

			if (hud.bossHealthIndex < *hud.bossHealth)
			{
				hud.bossHealthIndex = *hud.bossHealth;
			}
		}
	}
}

void drawHud()
{
	int i, x, y, h, w;
	float percentage, clipWidth;

	if (game.status == IN_INVENTORY)
	{
		drawBox(game.screen, (SCREEN_WIDTH - hud.itemBox->w) / 2, 15, hud.itemBox->w, hud.itemBox->h, 0, 0, 0);
	}

	drawSelectedInventoryItem((SCREEN_WIDTH - hud.itemBox->w) / 2, 15, hud.itemBox->w, hud.itemBox->h);

	drawImage(hud.itemBox, (SCREEN_WIDTH - hud.itemBox->w) / 2, 15, FALSE);

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
				drawImage(hud.heart, x, y, FALSE);
			}

			else if (i - 10 < percentage)
			{
				clipWidth = (percentage - (i - 10)) / 10;

				w = hud.heart->w * clipWidth;

				drawClippedImage(hud.heart, 0, 0, x, y, w, hud.heart->h);
			}

			drawImage(hud.emptyHeart, x, y, FALSE);

			x += hud.heart->w + 5;
		}
	}

	if (hud.infoMessage.surface != NULL)
	{
		drawImage(hud.infoMessage.surface, (SCREEN_WIDTH - hud.infoMessage.surface->w) / 2, SCREEN_HEIGHT - TILE_SIZE - 1, FALSE);
	}

	w = h = 5;

	for (i=0;i<player.maxHealth;i++)
	{
		if (i != 0 && (i % 15) == 0)
		{
			h += hud.heart->h;

			w = 5;
		}

		if (i < player.health)
		{
			drawImage(hud.heart, w, h, (player.health <= 3 && hud.thinkTime <= 30));
		}

		drawImage(hud.emptyHeart, w, h, FALSE);

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

	freeMessageQueue();
}

void setInfoBoxMessage(int thinkTime, char *fmt, ...)
{
	char text[MAX_MESSAGE_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	addMessageToQueue(text, thinkTime);
}

static void addMessageToQueue(char *text, int thinkTime)
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

	msg = (Message *)malloc(sizeof(Message));

	if (msg == NULL)
	{
		printf("Failed to allocate %d bytes for message queue\n", (int)sizeof(Message));

		exit(1);
	}

	STRNCPY(msg->text, text, sizeof(messageHead.text));

	msg->thinkTime = thinkTime;
	msg->next = NULL;

	head->next = msg;
}

static void getNextMessageFromQueue()
{
	Message *head = messageHead.next;

	if (head != NULL)
	{
		STRNCPY(hud.infoMessage.text, head->text, sizeof(hud.infoMessage.text));

		if (hud.infoMessage.surface != NULL)
		{
			SDL_FreeSurface(hud.infoMessage.surface);

			hud.infoMessage.surface = NULL;
		}

		hud.infoMessage.surface = generateTextSurface(hud.infoMessage.text, game.font, 255, 255, 255, 0, 0, 0);

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
