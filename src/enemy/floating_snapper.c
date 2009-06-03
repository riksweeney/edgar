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
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"

extern Entity *self;

static void floatUpAndDown(void);
static void snapShut(void);
static void snapShutFinish(void);
static void reopen(void);
static void reopenFinish(void);
static void touch(Entity *);
static void init(void);
static void wait(void);
static void trap(Entity *);

Entity *addFloatingSnapper(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Floating Snapper\n");

		exit(1);
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

	setEntityAnimation(e, STAND);

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
}

static void wait()
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
		}
	}

	else
	{
		self->thinkTime = self->maxThinkTime;
	}
}

static void touch(Entity *other)
{
	pushEntity(other);

	if (other->standingOn == self && other->type == PLAYER && self->health == 0)
	{
		self->health = 1;
	}

}

static void snapShut()
{
	floatUpAndDown();

	setEntityAnimation(self, ATTACK_1);

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
	}
}

static void snapShutFinish()
{
	floatUpAndDown();

	self->action = &snapShutFinish;

	setEntityAnimation(self, ATTACK_1);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &reopen;
	}

	self->health = 2;

	self->touch = &touch;
}

static void reopen()
{
	floatUpAndDown();

	self->action = &reopen;

	self->frameSpeed *= -1;

	setEntityAnimation(self, ATTACK_1);

	self->animationCallback = &reopenFinish;
}

static void reopenFinish()
{
	floatUpAndDown();

	setEntityAnimation(self, STAND);

	self->thinkTime = self->maxThinkTime;

	self->health = 0;

	self->action = &wait;

	self->touch = &touch;
}

static void init()
{
	self->action = self->health == 2 ? &snapShutFinish : &wait;

	self->action();
}
