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
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void entityWait(void);
static void init(void);
static void moveToTarget(void);
static void changeRandomTarget(Entity *);
static void creditsMove(void);

Entity *addFlyingBug(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Flying Bug");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &entityDie;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeRandomTarget;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	self->thinkTime = 0;

	if (self->mental == 0)
	{
		playSoundToMap("sound/enemy/bug/buzz", -1, self->x, self->y, 0);

		self->mental = 1;
	}

	self->action = &entityWait;

	self->action();
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		changeRandomTarget(NULL);

		self->action = &moveToTarget;
	}
}

static void changeRandomTarget(Entity *other)
{
	self->dirX = 0;
}

static void moveToTarget()
{
	float x, y;

	if (self->dirX == 0 || self->dirY == 0)
	{
		self->targetX = self->x + (prand() % 64) * (prand() % 2 == 0 ? -1 : 1);
		self->targetY = self->y + (prand() % 64) * (prand() % 2 == 0 ? -1 : 1);

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed;
		self->dirY *= self->speed;
	}

	x = self->dirX;
	y = self->dirY;

	checkToMap(self);

	if (atTarget())
	{
		self->thinkTime = 120 + (prand() % self->maxThinkTime);

		self->action = &entityWait;
	}

	else if (self->dirX != x || self->dirY != y)
	{
		changeRandomTarget(NULL);
	}
}

static void creditsMove()
{
	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}
