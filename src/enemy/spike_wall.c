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
#include "../world/target.h"

extern Entity *self;

static void init(void);
static void move(void);
static void moveStartEnd(void);
static void touch(Entity *other);
static void followTargets(void);

Entity *addSpikeWall(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Spike Wall");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->touch = &touch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case 0:
			self->action = &move;
		break;

		case 1:
			self->action = &moveStartEnd;
		break;

		case 2:
			self->action = &followTargets;
		break;
	}
}

static void followTargets()
{
	Target *t;
	char name[MAX_VALUE_LENGTH];

	checkToMap(self);

	if (fabs(self->x - self->startX) <= fabs(self->dirX) && fabs(self->y - self->startY) <= fabs(self->dirY))
	{
		self->x = self->startX;
		self->y = self->startY;

		self->thinkTime++;

		if (self->thinkTime > self->maxThinkTime)
		{
			self->thinkTime = 0;
		}

		self->dirX = 0;
		self->dirY = 0;
	}

	if (self->dirX == 0 && self->dirY == 0)
	{
		snprintf(name, sizeof(name), "%s_TARGET_%d", self->objectiveName, self->thinkTime);

		t = getTargetByName(name);

		if (t == NULL)
		{
			showErrorAndExit("Could not find target ", name);
		}

		self->startX = t->x;
		self->startY = t->y;

		if (self->x != self->startX)
		{
			self->dirX = self->startX < self->x ? -self->speed : self->speed;
		}

		if (self->y != self->startY)
		{
			self->dirY = self->startY < self->y ? -self->speed : self->speed;
		}
	}
}

static void moveStartEnd()
{
	float dirX;

	if (self->active == TRUE || self->health == 1)
	{
		dirX = self->dirX;

		self->health = 1;

		checkToMap(self);

		if (self->x <= self->startX || self->x >= self->endX)
		{
			self->x = self->x <= self->startX ? self->startX : self->endX;

			self->dirX = 0;
		}

		if (self->dirX == 0)
		{
			self->dirX = (dirX > 0 ? -self->speed : self->speed);
		}
	}

	else
	{
		checkToMap(self);
	}
}

static void move()
{
	float dirX;

	if (self->active == TRUE || self->health == 1)
	{
		dirX = self->dirX;

		self->health = 1;

		checkToMap(self);

		if (self->dirX == 0)
		{
			self->dirX = (dirX > 0 ? -self->speed : self->speed);
		}
	}

	else
	{
		checkToMap(self);
	}
}

static void touch(Entity *other)
{
	if (self->active == TRUE || self->health == 1)
	{
		entityTouch(other);
	}

	else
	{
		pushEntity(other);
	}
}
