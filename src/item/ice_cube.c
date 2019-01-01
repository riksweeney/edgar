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

extern Entity *self, player;

static void growFinish(void);
static void shrinkFinish(void);
static void iceFloat(void);
static void entityWait(void);
static void fallout(void);
static void platformTouch(Entity *);

Entity *addIceCube(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Ice Cube");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &entityWait;
	e->touch = &entityTouch;
	e->fallout = &fallout;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	checkToMap(self);

	if ((self->flags & ON_GROUND) || self->standingOn != NULL)
	{
		self->dirX = 0;
	}

	self->thinkTime--;

	if (self->thinkTime < 90)
	{
		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= NO_DRAW;
		}
	}

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}

static void fallout()
{
	if (self->environment == WATER)
	{
		self->x += self->w / 2;

		playSoundToMap("sound/common/freeze", -1, self->x, self->y, 0);

		loadProperties("item/ice_platform", self);

		self->element = WATER;

		setEntityAnimation(self, "ATTACK_1");

		self->x -= self->w / 2;

		self->animationCallback = &growFinish;

		self->thinkTime = 600;

		self->touch = &platformTouch;

		self->action = &iceFloat;

		self->dirX = self->dirY = 0;

		self->startX = 0;

		self->startY = self->y;

		self->flags |= FLY;
	}
}

static void platformTouch(Entity *other)
{
	if ((other->flags & ATTACKING) && other->type == ENEMY)
	{
		self->inUse = FALSE;
	}

	else
	{
		pushEntity(other);
	}
}

static void growFinish()
{
	setEntityAnimation(self, "ATTACK_2");
}

static void iceFloat()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "ATTACK_3");

		self->animationCallback = &shrinkFinish;
	}

	self->startX++;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 8;

	self->dirY = 3;
}

static void shrinkFinish()
{
	self->inUse = FALSE;
}
