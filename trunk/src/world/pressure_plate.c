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
#include "../hud.h"

extern Entity *self;
extern Game game;

static void wait(void);
static void activate(int);
static void touch(Entity *);

Entity *addPressurePlate(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("Couldn't get a free slot for a Pressure Plate!\n");

		exit(1);
	}

	loadProperties(name, e);

	e->touch = &touch;

	e->action = &wait;

	e->draw = &drawLoopingAnimationToMap;

	e->activate = &activate;

	e->x = x;
	e->y = y;

	e->health = 0;

	e->maxHealth = e->health;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	if (self->health != self->maxHealth)
	{
		setEntityAnimation(self, STAND);

		activateEntitiesWithName(self->objectiveName, FALSE);

		self->maxHealth = self->health;
	}

	self->health = 0;
}

static void touch(Entity *other)
{
	pushEntity(other);

	if (other->standingOn == self)
	{
		if (strlen(self->requires) == 0 || strcmpignorecase(self->requires, other->objectiveName) == 0)
		{
			activate(1);
		}

		else
		{
			setInfoBoxMessage(0,  _("%s is required to use this Pressure Plate"), self->requires);
		}
	}
}

static void activate(int val)
{
	self->health = val;

	if (self->health != self->maxHealth)
	{
		setEntityAnimation(self, WALK);

		activateEntitiesWithName(self->objectiveName, TRUE);

		self->maxHealth = self->health;
	}
}
