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

#include "../collisions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void init(void);
static void riseUp(void);
static void fallDown(void);
static void entityWait(void);
static void createBase(void);
static void baseWait(void);
static int drawBase(void);

Entity *addLavaGeyzer(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Lava Geyzer");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &pushEntity;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void createBase()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Lava Geyzer Base");
	}

	loadProperties("item/lava_geyzer_base", e);

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &baseWait;

	e->draw = &drawBase;

	e->head = self;

	setEntityAnimation(e, "STAND");
}

static void init()
{
	createBase();

	switch (self->mental)
	{
		case 0:
			self->action = &entityWait;
		break;

		case 1:
			self->action = &riseUp;
		break;

		default:
			self->action = &fallDown;
		break;
	}
}

static void entityWait()
{
	/* Bob up and down */

	self->health++;

	if (self->health >= 360)
	{
		self->health = 0;
	}

	self->y = self->endY + sin(DEG_TO_RAD(self->health)) * 12;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental = 1;

		self->action = &riseUp;
	}

	checkToMap(self);
}

static void riseUp()
{
	if (self->y == self->startY)
	{
		self->thinkTime--;

		checkToMap(self);

		if (self->thinkTime <= 0)
		{
			self->mental = 2;

			self->action = &fallDown;
		}
	}

	else
	{
		self->dirY = -self->speed;

		checkToMap(self);

		if (self->y <= self->startY)
		{
			self->y = self->startY;

			self->dirY = 0;

			self->thinkTime = 240;
		}
	}
}

static void fallDown()
{
	self->dirY = self->speed;

	checkToMap(self);

	if (self->y >= self->endY)
	{
		self->y = self->endY;

		self->dirY = 0;

		self->health = 0;

		self->thinkTime = self->maxThinkTime;

		self->mental = 0;

		self->action = &entityWait;
	}
}

static void baseWait()
{
	checkToMap(self);

	self->x = self->head->x;
	self->y = self->head->y + self->head->h;
}

static int drawBase()
{
	int y;

	drawLoopingAnimationToMap();

	y = self->head->endY + self->h;

	while (self->y < y)
	{
		drawSpriteToMap();

		self->y += self->h;
	}

	return TRUE;
}
