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
#include "../custom_actions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void entityWait(void);
static void init(void);
static void touch(Entity *);
static void addChain(void);
static void chainWait(void);
static int drawChain(void);
static void addRay(void);
static void rayWait(void);

Entity *addMagnet(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Magnet");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;
	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	addChain();

	addRay();

	self->endY = getMapFloor(self->startX, self->startY);

	self->action = &entityWait;
}

static void entityWait()
{
	float dirX;

	self->box.h = self->endY - self->startY;

	if (self->health == 0 && self->active == TRUE)
	{
		self->endX = playSoundToMap("sound/item/magnet", -1, self->x, self->y, -1);

		self->health = 1;
	}

	if (self->target != NULL)
	{
		if (self->active == TRUE)
		{
			setCustomAction(self->target, &helpless, 5, 0, 0);

			self->target->health = self->target->maxHealth * 100;

			if (self->target->y > self->y + self->h)
			{
				self->target->y -= 2;

				if (self->target->y <= self->y + self->h)
				{
					self->target->y = self->y + self->h;

					self->mental = 1;

					playSoundToMap("sound/common/dink", -1, self->x, self->y, 0);
				}
			}
		}

		else
		{
			self->target->health = self->target->maxHealth;

			self->target->flags &= ~FLY;

			self->target = NULL;

			self->mental = 0;

			self->health = 0;

			stopSound(self->endX);
		}
	}

	if (self->active == TRUE && self->mental == 1)
	{
		dirX = self->dirX;

		checkToMap(self);

		if (self->dirX == 0)
		{
			self->dirX = dirX < 0 ? self->speed : -self->speed;
		}

		if (self->target != NULL)
		{
			self->target->dirX = self->dirX;
		}
	}
}

static void touch(Entity *other)
{
	if (self->active == TRUE && self->target == NULL && strcmpignorecase(other->name, self->objectiveName) == 0)
	{
		if (abs(other->x - self->x - self->w / 2 + other->w / 2) <= 4)
		{
			self->target = other;

			setCustomAction(self->target, &helpless, 5, 0, 0);

			other->dirX = 0;

			other->flags |= FLY;
		}
	}
}

static void chainWait()
{
	self->x = self->head->x + self->head->w / 2 - self->w / 2;
	self->y = self->head->y - self->h;
}

static void addChain()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Magnet Chain");
	}

	loadProperties("item/grabber_chain", e);

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &chainWait;

	e->draw = &drawChain;

	e->head = self;

	setEntityAnimation(e, "STAND");
}

static int drawChain()
{
	int y;

	y = self->head->startY - self->h * 2;

	drawLoopingAnimationToMap();

	while (self->y >= y)
	{
		drawSpriteToMap();

		self->y -= self->h;
	}

	return TRUE;
}

static void addRay()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Magnet Ray");
	}

	loadProperties("item/magnet_ray", e);

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &rayWait;

	e->draw = &drawLoopingAnimationToMap;

	e->head = self;

	setEntityAnimation(e, "STAND");
}

static void rayWait()
{
	self->x = self->head->x + self->head->w / 2 - self->w / 2;
	self->y = self->head->y + self->head->h;

	if (self->head->active == TRUE)
	{
		self->flags &= ~NO_DRAW;
	}

	else
	{
		self->flags |= NO_DRAW;
	}
}
