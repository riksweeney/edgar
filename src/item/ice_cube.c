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

#include "../headers.h"

#include "../graphics/animation.h"
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "key_items.h"
#include "../system/random.h"
#include "../world/explosion.h"
#include "../system/error.h"

extern Entity *self, player;

static void growFinish(void);
static void shrinkFinish(void);
static void iceFloat(void);
static void wait(void);
static void fallout(void);

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

	e->action = &wait;
	e->touch = &entityTouch;
	e->fallout = &fallout;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

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

	checkToMap(self);
}

static void fallout()
{
	if (self->environment == WATER)
	{
		self->x += self->w / 2;

		playSoundToMap("sound/common/freeze.ogg", -1, self->x, self->y, 0);

		loadProperties("item/ice_platform", self);

		self->element = WATER;

		setEntityAnimation(self, ATTACK_1);

		self->x -= self->w / 2;

		self->animationCallback = &growFinish;

		self->thinkTime = 600;

		self->touch = &pushEntity;

		self->action = &iceFloat;

		self->dirX = self->dirY = 0;

		self->startX = 0;

		self->startY = self->y;

		self->flags |= FLY;
	}
}

static void growFinish()
{
	setEntityAnimation(self, ATTACK_2);
}

static void iceFloat()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, ATTACK_3);

		self->animationCallback = &shrinkFinish;
	}

	self->startX++;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 8;

	self->dirY = -3;
}

static void shrinkFinish()
{
	self->inUse = FALSE;
}
