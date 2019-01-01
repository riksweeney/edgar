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

static void largeRockFall(void);
static void smallRockFall(void);
static void shake(void);

extern Entity *self;

Entity *addLargeRock(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Large Rock");
	}

	loadProperties("common/large_rock", e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->action = &shake;
	e->fallout = &entityDieNoDrop;

	e->type = ENEMY;

	e->dirX = 2 * (prand() % 2 == 0 ? -1 : 1);

	e->x += e->dirX;

	e->dirY = 0;

	setEntityAnimation(e, "STAND");

	return e;
}

Entity *addSmallRock(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Small Rock");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;

	e->action = &smallRockFall;

	e->fallout = &entityDieNoDrop;

	e->type = TEMP_ITEM;

	e->dirY = 0;

	setEntityAnimation(e, "STAND");

	return e;
}

static void shake()
{
	self->dirY = 0;

	if (self->thinkTime % 4 == 0)
	{
		self->dirX *= -1;

		self->x += self->dirX * 2;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->x -= self->dirX;

		self->dirX = 0;

		self->action = &largeRockFall;
	}
}

static void largeRockFall()
{
	Entity *e;

	if (self->flags & ON_GROUND)
	{
		playSoundToMap("sound/common/rock_bounce", -1, self->x, self->y, 0);

		e = addSmallRock(self->x, self->y, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		e->dirX = -3;
		e->dirY = -8;

		e = addSmallRock(self->x, self->y, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		e->dirX = 3;
		e->dirY = -8;

		self->inUse = FALSE;
	}

	checkToMap(self);
}

static void smallRockFall()
{
	checkToMap(self);

	if (self->flags & ON_GROUND || self->standingOn != NULL)
	{
		self->inUse = FALSE;
	}
}
