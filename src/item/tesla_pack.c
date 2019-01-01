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
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/explosion.h"
#include "key_items.h"

extern Entity *self;

static void entityWait(void);
static void shockTarget(void);
static void shockEnd(void);
static void activate(int);
static void die(void);
static void shockWait(void);

Entity *addTeslaPack(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Tesla Pack");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &entityWait;

	e->touch = NULL;

	e->activate = &activate;

	e->die = &die;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	if (self->target != NULL)
	{
		self->layer = BACKGROUND_LAYER;

		self->x = self->target->x + self->target->w / 2;
		self->y = self->target->y + self->target->h / 2;

		self->x -= self->w / 2;
		self->y -= self->h / 2;

		if (self->target->startX <= 0)
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->action = &shockTarget;
			}
		}
	}

	else
	{
		self->layer = FOREGROUND_LAYER;

		checkToMap(self);
	}
}

static void shockTarget()
{
	Entity *temp, *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Tesla Shock");
	}

	loadProperties("item/tesla_shock", e);

	e->x = self->target->x;
	e->y = self->target->y;

	e->face = self->target->face;

	e->action = &shockWait;

	e->touch = NULL;

	e->activate = NULL;

	e->die = NULL;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	playSoundToMap("sound/item/tesla_electrocute", -1, self->x, self->y, 0);

	temp = self;

	self = self->target;

	self->flags &= ~INVULNERABLE;

	self->takeDamage(temp, 1000);

	self = temp;

	self->thinkTime = 120;

	self->action = &shockEnd;
}

static void shockEnd()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->target->health < 1000)
		{
			self->target = NULL;

			self->health = 0;

			self->mental = 20;

			self->action = &die;
		}

		else
		{
			self->target = NULL;

			self->health = 0;

			self->touch = &keyItemTouch;

			self->action = &entityWait;
		}

		loadProperties("item/tesla_pack_empty", self);
	}
}

static void activate(int val)
{
	if (self->target != NULL)
	{
		self->thinkTime = 30;

		addEntity(*self, self->target->x, self->target->y);

		self->inUse = FALSE;
	}
}

static void die()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addExplosion(self->x + prand() % self->w, self->y + prand() % self->h);

		e->damage = 0;

		e->touch = NULL;

		self->thinkTime = 8;

		self->mental--;

		if (self->mental <= 0)
		{
			self->inUse = FALSE;
		}
	}

	checkToMap(self);
}

static void shockWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}
