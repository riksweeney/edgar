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
#include "../system/random.h"
#include "../world/explosion.h"

extern Entity *self;

static void init(void);
static void die(void);
static void explode(void);
static void floatUpAndDown(void);

Entity *addExplodingSpikeSphere(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Exploding Spike Sphere");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->die = &die;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case 0:
			self->action = &moveLeftToRight;
		break;

		default:
			self->action = &floatUpAndDown;
		break;
	}
}

static void floatUpAndDown()
{
	self->endY += self->speed;

	if (self->endY >= 360)
	{
		self->endY = 0;
	}

	self->y = self->startY + cos(DEG_TO_RAD(self->endY)) * self->mental;
}

static void die()
{
	self->flags |= INVULNERABLE;

	self->dirX = 0;

	self->thinkTime--;

	if (self->thinkTime < 120)
	{
		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= FLASH;
		}
	}

	if (self->thinkTime <= 0)
	{
		self->health = 10;

		self->touch = NULL;

		self->action = &explode;
	}

	else
	{
		self->action = &die;
	}

	checkToMap(self);
}

static void explode()
{
	int x, y;
	Entity *e;

	self->flags |= NO_DRAW|FLY;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		x = self->x + self->w / 2;
		y = self->y + self->h / 2;

		x += (prand() % 32) * (prand() % 2 == 0 ? 1 : -1);
		y += (prand() % 32) * (prand() % 2 == 0 ? 1 : -1);

		e = addExplosion(x, y);

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		e->damage = 2;

		self->health--;

		self->thinkTime = 5;

		if (self->health == 0)
		{
			self->inUse = FALSE;
		}
	}

	self->action = &explode;
}
