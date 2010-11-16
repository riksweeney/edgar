/*
Copyright (C) 2009-2010 Parallel Realities

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
static void setAllBlockValues(int);
static void entityWait(void);
static void init(void);
static void die(void);
static void killAllBlocks(void);
static Entity *getFirstBlock(char *);

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

static void entityWait()
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
			e = self->target;

			/* Another number block */

			if (e != NULL)
			{
				e->health = 1;

				/*setEntityAnimation(e, e->health);*/
			}

			else
			{
                setInfoBoxMessage(60, 255, 255, 255, _("Complete"));

				activateEntitiesWithRequiredName(self->objectiveName, TRUE);

				if (self->damage == -1)
				{
					killAllBlocks();
				}
			}

			playSoundToMap("sound/item/number_block.ogg", -1, self->x, self->y, 0);

			self->health = 2;

			setEntityAnimation(self, self->health);
		}

		else if (self->health == 0)
		{
			setAllBlockValues(0);
		}
	}
}

static void init()
{
	Entity *first, *next;

	first = getFirstBlock(self->objectiveName);

	self->head = first;

	if (first->target == NULL)
	{
		next = getNextBlock(first->objectiveName, first->thinkTime + 1);

		while (next != NULL)
		{
			first->target = next;

			first = next;

			next = getNextBlock(first->objectiveName, first->thinkTime + 1);
		}
	}

	if (self->thinkTime == 0 && self->health != 2)
	{
		setAllBlockValues(0);
	}

	else
	{
		setEntityAnimation(self, self->health == 2 ? 2 : 0);
	}

	self->action = &entityWait;
}

static Entity *getFirstBlock(char *name)
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && entity[i].type == KEY_ITEM
			&& strcmpignorecase(entity[i].objectiveName, name) == 0 && entity[i].thinkTime == 0)
		{
			return &entity[i];
		}
	}

	showErrorAndExit("Could not find first Number Block for %s", entity[i].objectiveName);

	return NULL;
}

static void setAllBlockValues(int value)
{
	Entity *block = self->head;

	while (block != NULL)
	{
		block->health = (value == 0 && block->thinkTime == 0) ? 1 : value;

		setEntityAnimation(block, block->health == 1 ? 0 : block->health);

		block = block->target;
	}
}

static void killAllBlocks()
{
	Entity *block = self->head;

	while (block != NULL)
	{
		block->action = &die;

		block = block->target;
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
