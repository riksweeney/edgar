/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "../collisions.h"
#include "../system/error.h"

extern Entity *self;

static void entityWait(void);
static void init(void);
static void touch(Entity *);

Entity *addTreadMill(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Tread Mill");
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

	e->thinkTime = 0;

	return e;
}

static void init()
{
	char display[MAX_VALUE_LENGTH];
	Entity *e;

	snprintf(display, MAX_VALUE_LENGTH, "%s_METER", self->objectiveName);

	e = getEntityByObjectiveName(display);

	if (e == NULL)
	{
		showErrorAndExit("Tread Mill could not find meter %s", display);
	}

	self->target = e;

	self->action = &entityWait;
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->frameSpeed = 0;

		self->endX = 0;

		self->thinkTime = 0;
	}

	else
	{
		self->frameSpeed = self->endX;
	}
}

static void touch(Entity *other)
{
	pushEntity(other);

	if (other->standingOn == self)
	{
		self->thinkTime = 2;

		other->x -= other->dirX;

		self->endX = other->dirX;

		self->target->mental = self->endX * 2;
	}
}
