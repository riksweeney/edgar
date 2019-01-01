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

#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;
extern Game game;

static void entityWait(void);
static void touch(Entity *);

Entity *addTrapDoor(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Trap Door");
	}

	loadProperties(name, e);

	e->touch = &touch;

	e->action = &entityWait;

	e->draw = &drawLoopingAnimationToMap;

	e->x = x;
	e->y = y;

	e->maxHealth = e->health;

	e->flags |= OBSTACLE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	self->face = RIGHT;

	self->touch = self->active == FALSE ? &touch : NULL;

	self->action = &entityWait;

	setEntityAnimation(self, self->active == FALSE ? "STAND" : "WALK");
}

static void touch(Entity *other)
{
	pushEntity(other);
}
