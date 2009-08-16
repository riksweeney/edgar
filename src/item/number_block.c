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
#include "../hud.h"
#include "../inventory.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../collisions.h"

extern Entity *self, entity[MAX_ENTITIES];

static void touch(Entity *);
static void setAllBlockValues(char *, int);
static void wait(void);
static void init(void);

Entity *addNumberBlock(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Number Block\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->thinkTime = 0;

	return e;
}

static void wait()
{
	checkToMap(self);
}

static void touch(Entity *other)
{
	Entity *e;

	pushEntity(other);

	if (other->type == PLAYER && other->standingOn == self)
	{
		if (self->health == 1)
		{
			e = getEntityByRequiredName(self->requires);

			if (strcmpignorecase(e->name, self->name) == 0)
			{
				self->health = 2;

				e->health = 1;

				setEntityAnimation(self, self->health);

				setEntityAnimation(e, e->health);
			}

			else
			{
				activateEntitiesWithRequiredName(self->requires, self->active);

				setAllBlockValues(self->objectiveName, 2);
			}
		}

		else if (self->health == 0)
		{
			setAllBlockValues(self->objectiveName, 0);
		}
	}
}

static void init()
{
	setEntityAnimation(self, self->health);

	self->action = &wait;
}

static void setAllBlockValues(char *name, int value)
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && entity[i].type == KEY_ITEM && strcmpignorecase(entity[i].objectiveName, name) == 0)
		{
			entity[i].health = value == 0 && entity[i].thinkTime != 0 ? 1 : value;

			setEntityAnimation(self, self->health);
		}
	}
}
