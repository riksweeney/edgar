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

extern Entity *self;

static void die(void);
static void init(void);
static void changeTarget(Entity *);
static void flyToTarget(void);
static void creditsMove(void);

Entity *addWasp(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Wasp");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->takeDamage = &entityTakeDamageFlinch;
	e->reactToBlock = &changeTarget;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	self->targetX = self->endX;

	self->action = &flyToTarget;

	self->action();
}

static void flyToTarget()
{
	if (self->x == self->targetX || self->dirX == 0)
	{
		changeTarget(NULL);
	}

	self->face = (self->dirX > 0 ? RIGHT : LEFT);

	self->thinkTime += 5;

	self->dirY = cos(DEG_TO_RAD(self->thinkTime));

	if (abs(self->x - self->targetX) > self->speed)
	{
		self->dirX = (self->x < self->targetX ? self->speed : -self->speed);
	}

	else
	{
		self->x = self->targetX;

		self->dirX = 0;
	}

	checkToMap(self);
}

static void changeTarget(Entity *other)
{
	self->targetX = self->targetX == self->endX ? self->startX : self->endX;

	self->face = self->face == RIGHT ? LEFT : RIGHT;
}

static void creditsMove()
{
	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	self->thinkTime += 5;

	self->dirY = cos(DEG_TO_RAD(self->thinkTime));

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}

static void die()
{
	playSoundToMap("sound/enemy/wasp/wasp_die", -1, self->x, self->y, 0);

	entityDie();
}
