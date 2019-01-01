/*
Copyright (C) 2009-2019 Parallel Realities

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

#include "../headers.h"

#include "../audio/audio.h"
#include "../collisions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;
extern Input input;

static void entityWait(void);
static void activate(int);
static void init(void);
static void iconWait(void);
static void activateIcon(void);
static void addCursor(void);
static void cursorWait(void);
static void shuffle(void);
static void checkPairs(void);
static void iconWait(void);
static void iconInit(void);

Entity *addMemoryMatchDisplay(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Memory Match Display");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;
	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

Entity *addMemoryMatchIcon(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Memory Match Icon");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &iconInit;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	Entity *e;

	checkToMap(self);

	if (self->mental == -1)
	{
		shuffle();

		self->health = 6;

		self->mental = 0;

		self->targetX = 0;

		snprintf(self->requires, MAX_VALUE_LENGTH, _("Attempts remaining: %d"), self->health);
	}

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			e = self->target;

			while (e != NULL)
			{
				if (e->mental == 1)
				{
					e->mental = 0;

					setEntityAnimation(e, "STAND");
				}

				e = e->target;
			}

			if (self->health <= 0)
			{
				self->mental = 1;
			}
		}
	}

	if (self->mental == 0)
	{
		setInfoBoxMessage(0, 255, 255, 255, self->requires);
	}
}

static void activate(int val)
{
	if (self->thinkTime <= 0)
	{
		if (val == 1)
		{
			self->endX -= TILE_SIZE;

			if (self->endX < self->startX)
			{
				self->endX = self->startX;
			}

			else
			{
				playSoundToMap("sound/common/click", -1, self->x, self->y, 0);
			}
		}

		else if (val == 2)
		{
			self->endX += TILE_SIZE;

			if (self->endX >= self->startX + self->w)
			{
				self->endX = self->startX + self->w - TILE_SIZE;
			}

			else
			{
				playSoundToMap("sound/common/click", -1, self->x, self->y, 0);
			}
		}

		else if (val == 3)
		{
			self->endY -= TILE_SIZE;

			if (self->endY < self->startY)
			{
				self->endY = self->startY;
			}

			else
			{
				playSoundToMap("sound/common/click", -1, self->x, self->y, 0);
			}
		}

		else if (val == 4)
		{
			self->endY += TILE_SIZE;

			if (self->endY >= self->startY + self->h)
			{
				self->endY = self->startY + self->h - TILE_SIZE;
			}

			else
			{
				playSoundToMap("sound/common/click", -1, self->x, self->y, 0);
			}
		}

		else if (val == 5)
		{
			playSoundToMap("sound/common/click", -1, self->x, self->y, 0);

			activateIcon();
		}
	}
}

static void init()
{
	int i, x, y;
	Entity *e, *prev;

	prev = self;

	x = self->x;
	y = self->y;

	if (self->mental != 2)
	{
		for (i=0;i<16;i++)
		{
			/* 4 x 4 grid */

			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add a Memory Match Icon");
			}

			loadProperties("item/mastermind_peg", e);

			e->mental = 0;

			setEntityAnimation(e, "STAND");

			if (i != 0 && i % 4 == 0)
			{
				x = self->x;
				y += TILE_SIZE;
			}

			else if (i != 0)
			{
				x += TILE_SIZE;
			}

			e->face = RIGHT;

			e->x = x;
			e->y = y;

			e->action = &iconWait;

			e->draw = &drawLoopingAnimationToMap;

			prev->target = e;

			prev = e;

			e->target = NULL;
		}

		shuffle();

		addCursor();
	}

	self->action = &entityWait;
}

static void iconWait()
{

}

static void activateIcon()
{
	Entity *e;

	e = self->target;

	while (e != NULL)
	{
		if (e->x == self->endX && e->y == self->endY)
		{
			if (e->mental == 0)
			{
				e->mental = 1;

				setEntityAnimationByID(e, e->health);

				self->targetX++;

				if (self->targetX == 2)
				{
					checkPairs();

					self->targetX = 0;
				}
			}

			break;
		}

		e = e->target;
	}
}

static void checkPairs()
{
	int colour1, colour2, success;
	Entity *e;

	e = self->target;

	colour2 = colour1 = -1;

	while (e != NULL)
	{
		if (e->mental == 1)
		{
			if (colour1 == -1)
			{
				colour1 = e->health;
			}

			else
			{
				colour2 = e->health;

				break;
			}
		}

		e = e->target;
	}

	success = colour1 == colour2 ? TRUE : FALSE;

	e = self->target;

	colour1 = TRUE;

	while (e != NULL)
	{
		if (e->mental == 1)
		{
			if (success == TRUE)
			{
				e->mental = 2;
			}
		}

		else if (e->mental == 0)
		{
			colour1 = FALSE;
		}

		e = e->target;
	}

	if (colour1 == TRUE)
	{
		e = self->target;

		while (e != NULL)
		{
			e->flags &= ~DO_NOT_PERSIST;

			e = e->target;
		}

		self->mental = 2;
	}

	if (success == FALSE)
	{
		self->thinkTime = 30;

		self->health--;

		snprintf(self->requires, MAX_VALUE_LENGTH, _("Attempts remaining: %d"), self->health);
	}
}

static void addCursor()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Memory Match Cursor");
	}

	loadProperties("item/jigsaw_puzzle_cursor", e);

	e->x = self->endX;
	e->y = self->endY;

	e->action = &cursorWait;

	e->draw = &drawLoopingAnimationToMap;

	e->head = self;

	setEntityAnimation(e, "STAND");
}

static void cursorWait()
{
	self->x = self->head->endX;
	self->y = self->head->endY;
}

static void shuffle()
{
	int i, j, colour;
	Entity *e1, *e2;

	e1 = self->target;

	i = 0;

	j = 0;

	colour = 1;

	/* Place the 5 colours in order */

	while (e1 != NULL)
	{
		e1->health = colour;

		e1->mental = 0;

		i++;

		if (i == 2)
		{
			colour++;

			/* Randomly pick a few more */

			if (colour == 6 || j == 1)
			{
				colour = 1 + prand() % 5;

				j = 1;
			}

			i = 0;
		}

		setEntityAnimation(e1, "STAND");

		e1 = e1->target;
	}

	/* Now loop through the icons and switch the colours */

	e1 = self->target;

	i = 0;

	while (e1 != NULL)
	{
		j = prand() % 16;

		e2 = self->target;

		i = 0;

		while (e2 != NULL)
		{
			if (i == j)
			{
				colour = e1->health;

				e1->health = e2->health;

				e2->health = colour;

				break;
			}

			i++;

			e2 = e2->target;
		}

		e1 = e1->target;
	}

	self->endX = self->x;
	self->endY = self->y;
}

static void iconInit()
{
	setEntityAnimationByID(self, self->health);

	self->mental = 0;

	self->action = &entityWait;

	self->flags &= ~DO_NOT_PERSIST;
}
