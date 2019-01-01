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
#include "../entity.h"
#include "../graphics/animation.h"
#include "../inventory.h"
#include "../medal.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;
extern Game game;

static void floatUpAndDown(void);
static void snapShut(void);
static void snapShutFinish(void);
static void reopen(void);
static void reopenFinish(void);
static void touch(Entity *);
static void init(void);
static void entityWait(void);
static void trap(Entity *);

Entity *addFloatingSnapper(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Floating Snapper");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->die = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void floatUpAndDown()
{
	self->endX++;

	if (self->endX >= 360)
	{
		self->endX = 0;
	}

	self->y = self->endY + sin(DEG_TO_RAD(self->endX)) * 5;

	self->dirY = 3;
}

static void entityWait()
{
	floatUpAndDown();

	if (self->maxThinkTime < 0)
	{
		self->health = 1;

		if (self->thinkTime < 0)
		{
			self->thinkTime = abs(self->maxThinkTime);
		}
	}

	if (self->health == 1)
	{
		self->thinkTime--;

		if (self->x == self->startX || (self->thinkTime % 4 == 0))
		{
			self->x = self->startX + (3 * (self->x < self->startX ? 1 : -1));
		}

		if (self->thinkTime <= 0)
		{
			self->x = self->startX;

			self->action = &snapShut;

			playSoundToMap("sound/enemy/floating_snapper/chomp", -1, self->x, self->y, 0);
		}
	}

	else
	{
		self->thinkTime = self->maxThinkTime;
	}
}

static void touch(Entity *other)
{
	if (strcmpignorecase(other->name, self->name) != 0)
	{
		pushEntity(other);
	}

	if (other->standingOn == self && other->type == PLAYER && self->health == 0)
	{
		self->health = 1;
	}

}

static void snapShut()
{
	floatUpAndDown();

	setEntityAnimation(self, "ATTACK_1");

	self->touch = &trap;

	self->animationCallback = &snapShutFinish;

	self->thinkTime = 180;
}

static void trap(Entity *other)
{
	if (other->type == PLAYER)
	{
		other->flags |= NO_DRAW;

		other->fallout();

		if (other->health == 1 && getInventoryItemByObjectiveName("Amulet of Resurrection") == NULL)
		{
			self->health = 5;

			other->flags |= FLY;
		}

		game.timesEaten++;

		if (game.timesEaten == 5)
		{
			addMedal("eaten_5");
		}
	}
}

static void snapShutFinish()
{
	floatUpAndDown();

	self->action = &snapShutFinish;

	setEntityAnimation(self, "ATTACK_1");

	if (self->health != 5)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &reopen;
		}

		self->health = 2;

		self->touch = &touch;
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			playSoundToMap("sound/enemy/floating_snapper/burp", -1, self->x, self->y, 0);
		}
	}
}

static void reopen()
{
	floatUpAndDown();

	self->action = &reopen;

	self->frameSpeed *= -1;

	setEntityAnimation(self, "ATTACK_1");

	self->animationCallback = &reopenFinish;
}

static void reopenFinish()
{
	floatUpAndDown();

	setEntityAnimation(self, "STAND");

	self->thinkTime = self->maxThinkTime;

	self->health = 0;

	self->action = &entityWait;

	self->touch = &touch;
}

static void init()
{
	self->action = self->health == 2 ? &snapShutFinish : &entityWait;

	self->action();
}
