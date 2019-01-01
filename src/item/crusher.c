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

static void crush(void);
static void touch(Entity *);
static void armWait(void);
static int drawArm(void);
static void createArm(void);
static void init(void);

Entity *addCrusher(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Crusher");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void touch(Entity *other)
{
	Entity *temp;

	/* Injure enemies as well as player */

	if (other->type == ENEMY)
	{
		if (other->takeDamage != NULL && !(other->flags & INVULNERABLE))
		{
			temp = self;

			self = other;

			self->takeDamage(temp, 10);

			self = temp;
		}
	}

	else
	{
		entityTouch(other);
	}
}

static void init()
{
	createArm();

	self->flags |= ATTACKING;

	self->action = &crush;
}

static void crush()
{
	if (self->thinkTime > 0)
	{
		if (self->active == TRUE || !(self->active == FALSE && self->y == self->startY))
		{
			self->thinkTime--;
		}
	}

	else
	{
		self->y += self->mental == 1 ? self->speed : -self->speed;

		if (self->y >= self->endY || self->standingOn != NULL)
		{
			self->y = self->endY;

			self->mental = 2;

			self->thinkTime = 30;

			if (self->health == -1)
			{
				playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);
			}
		}

		else if (self->y <= self->startY)
		{
			self->y = self->startY;

			self->mental = 1;

			self->thinkTime = self->maxThinkTime;
		}
	}
}

static void createArm()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Crusher Arm");
	}

	loadProperties("item/crusher_arm", e);

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &armWait;

	e->draw = &drawArm;

	e->head = self;

	setEntityAnimation(e, "STAND");
}

static void armWait()
{
	checkToMap(self);

	self->x = self->head->x;
	self->y = self->head->y - self->h;
}

static int drawArm()
{
	int y;

	y = self->head->startY - self->h * 2;

	drawLoopingAnimationToMap();

	while (self->y >= y)
	{
		drawSpriteToMap();

		self->y -= self->h;
	}

	return TRUE;
}
