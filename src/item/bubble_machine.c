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
#include "bubble.h"

extern Entity *self;

static void entityWait(void);
static void touch(Entity *);
static void init(void);

Entity *addBubbleMachine(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Bubble Machine");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;
	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->health == 3)
		{
			setEntityAnimation(self, "STAND");

			self->health = 0;
		}

		else
		{
			setEntityAnimationByID(self, self->health);
		}

		self->thinkTime = 0;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	Entity *e;

	pushEntity(other);

	if (other->standingOn == self)
	{
		if (self->thinkTime < 5)
		{
			playSoundToMap("sound/item/inflate", -1, self->x, self->y, 0);

			self->health++;
		}

		if (self->health == 3)
		{
			if (self->thinkTime < 5)
			{
				e = addBubble(self->x, self->y, "item/bubble");

				e->x = self->face == LEFT ? getLeftEdge(self) : getRightEdge(self);

				e->x += self->face == LEFT ? -e->w : e->w;

				e->y = self->y + self->h / 2 - e->h / 2;

				e->dirX = self->face == LEFT ? -e->speed : e->speed;
			}

			self->thinkTime = 60;
		}

		else if (self->health > 3)
		{
			self->health = 3;

			self->thinkTime = 60;
		}

		else
		{
			self->thinkTime = 10;
		}

		setEntityAnimationByID(self, self->health + 3);
	}
}

static void init()
{
	setEntityAnimationByID(self, self->health);

	self->action = &entityWait;
}
