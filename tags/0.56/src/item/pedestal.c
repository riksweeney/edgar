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
#include "../inventory.h"
#include "../hud.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "key_items.h"
#include "../event/script.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../system/error.h"

extern Entity *self, entity[MAX_ENTITIES];

static void wait(void);
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

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
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

			removeInventoryItem(e->objectiveName);

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
	if (self->target != NULL)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to retrieve %s"), self->target->objectiveName);
	}

	else
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
	}
}

static void validate()
{
	int i, status;

	status = 0;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && entity[i].type == KEY_ITEM
			&& strcmpignorecase(entity[i].name, self->objectiveName) == 0)
		{
			if (entity[i].target == NULL)
			{
				status = 1;

				break;
			}

			else if (strcmpignorecase(entity[i].requires, entity[i].target->objectiveName) != 0)
			{
				status = 2;

				break;
			}
		}
	}

	if (status == 0)
	{
		for (i=0;i<MAX_ENTITIES;i++)
		{
			if (entity[i].inUse == TRUE && entity[i].type == KEY_ITEM
				&& strcmpignorecase(entity[i].name, self->objectiveName) == 0)
			{
				entity[i].active = FALSE;
			}
		}

		fireTrigger(self->name);

		fireGlobalTrigger(self->name);
	}

	else if (status == 2)
	{
		runScript("wrong_order");
	}
}

static void init()
{
	int i;
	Entity *e = NULL;

	if (strlen(self->objectiveName) != 0)
	{
		for (i=0;i<MAX_ENTITIES;i++)
		{
			if (entity[i].inUse == TRUE && entity[i].type == ITEM
				&& strcmpignorecase(entity[i].objectiveName, self->objectiveName) == 0)
			{
				e = &entity[i];

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
	}

	self->action = &wait;

	self->touch = self->active == TRUE ? &touch : NULL;
}

static void statueWait()
{

}
