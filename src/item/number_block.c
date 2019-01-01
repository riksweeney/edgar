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
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

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

	setEntityAnimation(e, "STAND");

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

			playSoundToMap("sound/item/number_block", -1, self->x, self->y, 0);

			self->health = 2;

			setEntityAnimationByID(self, self->health);
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
		setEntityAnimationByID(self, self->health == 2 ? 2 : 0);
	}

	self->action = &entityWait;
}

static Entity *getFirstBlock(char *name)
{
	EntityList *el, *entities;

	entities = getEntities();

	for (el=entities->next;el!=NULL;el=el->next)
	{
		if (el->entity->inUse == TRUE && el->entity->type == KEY_ITEM
			&& strcmpignorecase(el->entity->objectiveName, name) == 0 && el->entity->thinkTime == 0)
		{
			return el->entity;
		}
	}

	showErrorAndExit("Could not find first Number Block for %s", el->entity->objectiveName);

	return NULL;
}

static void setAllBlockValues(int value)
{
	Entity *block = self->head;

	while (block != NULL)
	{
		block->health = (value == 0 && block->thinkTime == 0) ? 1 : value;

		setEntityAnimationByID(block, block->health == 1 ? 0 : block->health);

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
	EntityList *el, *entities;

	entities = getEntities();

	for (el=entities->next;el!=NULL;el=el->next)
	{
		if (el->entity->inUse == TRUE && el->entity->type == KEY_ITEM && strcmpignorecase(el->entity->objectiveName, name) == 0
			&& el->entity->thinkTime == value)
		{
			return el->entity;
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
