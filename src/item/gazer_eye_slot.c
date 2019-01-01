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
#include "../event/trigger.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../inventory.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void sink(void);
static void activate(int);
static void entityWait(void);
static void init(void);
static void touch(Entity *);

Entity *addGazerEyeSlot(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Gazer Eye Slot");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->activate = &activate;
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

static void sink()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;

		self->y++;

		if (self->y >= self->endY)
		{
			self->inUse = FALSE;
		}
	}
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && self->active == FALSE)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
	}
}

static void activate(int val)
{
	if (self->active == FALSE)
	{
		if (removeInventoryItemByObjectiveName(self->requires) == TRUE)
		{
			self->active = TRUE;

			self->thinkTime = 60;

			fireTrigger(self->objectiveName);

			fireGlobalTrigger(self->objectiveName);

			setEntityAnimation(self, "WALK");

			self->action = &sink;

			setInfoBoxMessage(60, 255, 255, 255, _("Used %s"), _(self->requires));
		}

		else
		{
			setInfoBoxMessage(60, 255, 255, 255, _("%s is required"), _(self->requires));
		}
	}
}

static void init()
{
	if (self->active == TRUE)
	{
		setEntityAnimation(self, "WALK");

		self->action = &sink;
	}

	else
	{
		setEntityAnimation(self, "STAND");

		self->action = &entityWait;
	}
}
