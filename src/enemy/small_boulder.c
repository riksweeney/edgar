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
#include "../item/item.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void roll(void);
static void die(void);
static void dieInLava(void);
static void fallout(void);

Entity *addSmallBoulder(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Small Boulder");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &entityTouch;

	e->die = &die;

	e->fallout = &fallout;

	e->action = &roll;

	e->type = ENEMY;

	e->dirY = 0;

	setEntityAnimation(e, "STAND");

	return e;
}

static void roll()
{
	float dirX = self->dirX;
	long onGround;

	self->thinkTime--;

	if (!(self->flags & ON_GROUND) && self->mental == 0)
	{
		self->frameSpeed = 0;

		self->dirX = 0;
	}

	else if (self->thinkTime <= 0)
	{
		self->dirX = (self->face == LEFT ? -self->speed : self->speed);

		self->frameSpeed = 1;

		if (self->health != 1)
		{
			self->targetX = playSoundToMap("sound/boss/boulder_boss/roll", -1, self->x, self->y, -1);

			self->health = 1;
		}
	}

	onGround = (self->flags & ON_GROUND);

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (onGround == 0)
		{
			playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);
		}

		self->mental = 1;
	}

	if (self->dirX == 0 && dirX != 0 && self->mental == 1 && self->element == AIR)
	{
		self->die();
	}

	syncBoulderFrameSpeed();
}

static void die()
{
	int i;
	Entity *e;

	for (i=0;i<4;i++)
	{
		e = addTemporaryItem("misc/small_boulder_piece", self->x, self->y, self->face, 0, 0);

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->thinkTime = 60 + (prand() % 60);

		stopSound(self->targetX);
	}

	playSoundToMap("sound/common/rock_shatter", -1, self->x, self->y, 0);

	self->inUse = FALSE;
}


static void fallout()
{
	self->element = FIRE;

	self->dirX = 0;

	self->dirY = 0;

	self->frameSpeed = 0;

	self->flags |= DO_NOT_PERSIST;

	self->action = &dieInLava;
}

static void dieInLava()
{
	self->dirY = 1.5;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		stopSound(self->targetX);

		self->inUse = FALSE;
	}
}
