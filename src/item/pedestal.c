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

#include "../collisions.h"
#include "../entity.h"
#include "../event/global_trigger.h"
#include "../event/script.h"
#include "../event/trigger.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../inventory.h"
#include "../medal.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void entityWait(void);
static void init(void);
static void statueWait(void);
static void addStatue(int);
static void touch(Entity *);
static void validate(void);

Entity *addPedestal(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Pedestal");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &touch;
	e->activate = &addStatue;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	checkToMap(self);
}

static void addStatue(int val)
{
	Entity *e;

	if (self->target == NULL)
	{
		e = getCurrentInventoryItem();

		if (e == NULL || strstr(e->name, "_statue") == NULL)
		{
			runScript("statue_required");
		}

		else
		{
			STRNCPY(self->objectiveName, e->objectiveName, sizeof(e->objectiveName));

			e = addEntity(*e, self->x, self->y);

			self->target = e;

			self->target->x = self->x + self->w / 2;

			self->target->x -= self->target->w / 2;

			self->target->y = self->y - self->target->h;

			self->target->flags |= FLY;

			self->target->touch = NULL;

			self->target->action = &statueWait;

			removeInventoryItemByObjectiveName(e->objectiveName);

			validate();
		}
	}

	else
	{
		e = addEntity(*self->target, 0, 0);

		addToInventory(e);

		self->target->inUse = FALSE;

		self->target = NULL;

		self->objectiveName[0] = '\0';
	}
}

static void touch(Entity *other)
{
	if (self->active == TRUE)
	{
		if (self->target != NULL)
		{
			setInfoBoxMessage(0, 255, 255, 255, _("Press Action to retrieve %s"), self->target->objectiveName);
		}

		else
		{
			setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
		}
	}
}

static void validate()
{
	int allStatues, orderCorrect;
	EntityList *list = getEntitiesByName(self->name);
	EntityList *l;
	Entity *e;

	allStatues = TRUE;

	orderCorrect = TRUE;

	for (l=list->next;l!=NULL;l=l->next)
	{
		e = l->entity;

		if (e->target == NULL)
		{
			allStatues = FALSE;

			break;
		}

		else if (strcmpignorecase(e->requires, e->target->objectiveName) != 0)
		{
			orderCorrect = FALSE;
		}
	}

	if (allStatues == TRUE)
	{
		if (orderCorrect == TRUE)
		{
			for (l=list->next;l!=NULL;l=l->next)
			{
				e = l->entity;

				e->active = FALSE;

				e->activate = NULL;
			}

			fireTrigger(self->name);

			fireGlobalTrigger(self->name);

			addMedal("occult");
		}

		else
		{
			runScript("wrong_order");
		}
	}

	freeEntityList(list);
}

static void init()
{
	EntityList *list;
	EntityList *l;
	Entity *e = NULL;

	if (strlen(self->objectiveName) != 0)
	{
		list = getEntitiesByObjectiveName(self->objectiveName);

		for (l=list->next;l!=NULL;l=l->next)
		{
			e = l->entity;

			if (e != self && strcmpignorecase(self->objectiveName, e->objectiveName) == 0)
			{
				break;
			}
		}

		if (e == NULL)
		{
			showErrorAndExit("Pedestal could not get statue %s", self->objectiveName);
		}

		self->target = e;

		self->target->x = self->x + self->w / 2;

		self->target->x -= self->target->w / 2;

		self->target->y = self->y - self->target->h;

		self->target->flags |= FLY;

		self->target->touch = NULL;

		self->target->action = &statueWait;

		freeEntityList(list);
	}

	self->action = &entityWait;

	self->activate = self->active == TRUE ? &addStatue : NULL;
}

static void statueWait()
{

}
