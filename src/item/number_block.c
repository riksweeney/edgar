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
#include "../system/error.h"

extern Entity *self, entity[MAX_ENTITIES];

static void touch(Entity *);
static Entity *getNextBlock(char *, int);
static void setAllBlockValues(char *, int);
static void wait(void);
static void init(void);
static void die(void);
static void killAllBlocks(char *);

Entity *addNumberBlock(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Number Block");
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
			e = getNextBlock(self->objectiveName, self->thinkTime + 1);

			/* Another number block */

			if (e != NULL)
			{
				e->health = 1;

				/*setEntityAnimation(e, e->health);*/
			}

			else
			{
                setInfoBoxMessage(60, _("Complete"));
                
				activateEntitiesWithRequiredName(self->objectiveName, TRUE);

				if (self->damage == -1)
				{
					killAllBlocks(self->objectiveName);
				}
			}

			playSoundToMap("sound/item/number_block.ogg", -1, self->x, self->y, 0);

			self->health = 2;

			setEntityAnimation(self, self->health);
		}

		else if (self->health == 0)
		{
			setAllBlockValues(self->objectiveName, 0);
		}
	}
}

static void init()
{
	if (self->thinkTime == 0 && self->health != 2)
	{
		setAllBlockValues(self->objectiveName, 0);
	}

	else
	{
		setEntityAnimation(self, self->health == 2 ? 2 : 0);
	}

	self->action = &wait;
}

static void setAllBlockValues(char *name, int value)
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && entity[i].type == KEY_ITEM && strcmpignorecase(entity[i].objectiveName, name) == 0)
		{
			entity[i].health = (value == 0 && entity[i].thinkTime == 0) ? 1 : value;

			setEntityAnimation(&entity[i], entity[i].health == 1 ? 0 : entity[i].health);
		}
	}
}

static void killAllBlocks(char *name)
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && entity[i].type == KEY_ITEM && strcmpignorecase(entity[i].objectiveName, name) == 0)
		{
			entity[i].action = &die;
		}
	}
}

static Entity *getNextBlock(char *name, int value)
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && entity[i].type == KEY_ITEM && strcmpignorecase(entity[i].objectiveName, name) == 0
			&& entity[i].thinkTime == value)
		{
			return &entity[i];
		}
	}

	return NULL;
}

static void die()
{
	self->flags |= DO_NOT_PERSIST;

	self->alpha -= 3;

	if (self->alpha <= 0)
	{
		self->inUse = FALSE;
	}
}
