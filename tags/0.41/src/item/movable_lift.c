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
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "key_items.h"
#include "item.h"
#include "../system/error.h"
#include "../hud.h"

extern Entity *self;

static void touch(Entity *);
static void platformWait(void);
static void activate(int);
static void addPlatform(void);

Entity *addMovableLift(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Movable Lift");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &addPlatform;
	e->touch = &pushEntity;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->thinkTime = 0;

	return e;
}

static void touch(Entity *other)
{
	pushEntity(other);

	if (other->standingOn == self)
	{
		setInfoBoxMessage(0, _("Push Up or Down to raise or lower the platform"));

		self->thinkTime = 120;
	}
}

static void platformWait()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			self->health = 0;
		}
	}

	self->x = self->head->x + self->w / 2;

	self->x -= self->w / 2;

	if (self->y != self->targetY)
	{
		if (self->y < self->targetY)
		{
			self->y += 4;

			if (self->y >= self->targetY)
			{
				self->y = self->targetY;
			}
		}

		else if (self->y > self->targetY)
		{
			self->y -= 4;

			if (self->y <= self->targetY)
			{
				self->y = self->targetY;
			}
		}
	}
}

static void activate(int val)
{
	self->health += val;

	if (self->health < 0)
	{
		self->health = 0;

		self->targetY = self->head->y - self->h;
	}

	else if (self->health > 1)
	{
		self->health = 1;

		self->targetY = self->head->y - self->h - 64;
	}

	self->thinkTime = 120;
}

static void addPlatform()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Movable Lift Platform");
	}

	loadProperties("item/movable_lift_platform", e);

	e->x = self->x + self->w / 2;
	e->y = self->y;

	e->x -= e->w / 2;
	e->y -= e->h;

	e->face = self->face;

	e->action = &platformWait;
	e->touch = &touch;
    e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->health = 0;

	e->thinkTime = 0;

	e->head = self;

	self->action = &platformWait;
}
