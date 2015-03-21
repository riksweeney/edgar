/*
Copyright (C) 2009-2015 Parallel Realities

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
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "item.h"

extern Entity *self;

static void touch(Entity *);
static void activate(int);
static void init(void);

Entity *addMoveableLift(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Moveable Lift");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &touch;
	e->activate = &activate;
	e->fallout = &itemFallout;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	e->thinkTime = 0;

	return e;
}

static void init()
{
	if (self->health < 0)
	{
		self->health = 0;
	}

	else if (self->health > 2)
	{
		self->health = 2;
	}

	setEntityAnimationByID(self, self->health);

	checkToMap(self);

	self->action = &doNothing;
}

static void touch(Entity *other)
{
	pushEntity(other);

	if (other->standingOn == self)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Push Up or Down to raise or lower the platform"));

		self->thinkTime = 120;
	}
}

static void activate(int val)
{
	self->health += val;

	if (self->health < 0)
	{
		self->health = 0;
	}

	else if (self->health > 2)
	{
		self->health = 2;
	}

	setEntityAnimationByID(self, self->health);

	self->thinkTime = 120;
}
