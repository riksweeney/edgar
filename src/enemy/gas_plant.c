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

extern Entity *self;

static void entityWait(void);
static void takeDamage(Entity *, int);
static void init(void);
static void sprayGas(void);
static void sprayGasWait(void);
static void sprayMove(void);

Entity *addGasPlant(int x, int y, char *name)
{
	int frameCount;
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Gas Plant");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->touch = &entityTouch;
	e->takeDamage = &takeDamage;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	frameCount = getFrameCount(e);

	e->currentFrame = prand() % frameCount;

	return e;
}

static void init()
{
	if (self->mental >= 0)
	{
		self->mental = 0;

		self->action = &entityWait;
	}

	else
	{
		switch (self->mental)
		{
			case -2:
				self->action = &sprayGas;
			break;

			case -3:
				self->action = &sprayGasWait;
			break;

			default:
				self->action = &entityWait;
			break;
		}
	}
}

static void entityWait()
{
	checkToMap(self);

	if (self->mental == -1)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->mental = -2;

			self->action = &sprayGas;
		}
	}
}

static void takeDamage(Entity *other, int damage)
{
	entityTakeDamageNoFlinch(other, damage);

	if (self->health > 0 && self->mental == 0)
	{
		self->mental = 1;

		self->thinkTime = 15;

		self->action = &sprayGas;
	}
}

static void sprayGas()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "ATTACK_1");

		for (i=0;i<2;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add a Gas Plant Spray");
			}

			loadProperties("enemy/gas_plant_spray", e);

			e->action = &sprayMove;
			e->draw = &drawLoopingAnimationToMap;
			e->touch = &entityTouch;

			e->type = ENEMY;

			e->dirX = i == 0 ? -e->speed : e->speed;

			e->x = self->x + self->w / 2 - e->w / 2;
			e->y = self->y + e->offsetY;
		}

		playSoundToMap("sound/item/spray", -1, self->x, self->y, 0);

		self->mental = self->mental >= 0 ? 0 : -3;

		self->action = &sprayGasWait;

		self->thinkTime = 30;
	}

	checkToMap(self);
}

static void sprayGasWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		if (self->mental < 0)
		{
			self->thinkTime = self->maxThinkTime;

			self->mental = -1;
		}

		self->action = &entityWait;
	}

	checkToMap(self);
}

static void sprayMove()
{
	self->dirX *= 0.98;

	if (self->mental < 2)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->mental++;

			setEntityAnimation(self, self->mental == 1 ? "WALK" : "JUMP");

			self->thinkTime = self->mental == 2 ? 180 : 30;
		}
	}

	if (fabs(self->dirX) <= 0.05)
	{
		self->dirX = 0;
	}

	checkToMap(self);

	if (self->mental == 2)
	{
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
}
