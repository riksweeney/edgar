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
#include "../event/global_trigger.h"
#include "../event/trigger.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../inventory.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void touch(Entity *);
static void activate(int);
static void entityWait(void);
static void init(void);

Entity *addPowerGenerator(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Power Generator");
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

	if (self->health < 0)
	{
		if (self->active == TRUE)
		{
			self->health *= -1;

			fireTrigger(self->objectiveName);

			fireGlobalTrigger(self->objectiveName);

			playSoundToMap("sound/item/generator", -1, self->x, self->y, 0);

			setEntityAnimationByID(self, self->health);

			self->frameSpeed = 1;
		}
	}
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && self->active == FALSE && self->health > 0)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
	}
}

static void activate(int val)
{
	if (self->active == FALSE && self->health > 0)
	{
		if (removeInventoryItemByObjectiveName(self->requires) == TRUE)
		{
			self->active = TRUE;

			fireTrigger(self->objectiveName);

			fireGlobalTrigger(self->objectiveName);

			playSoundToMap("sound/item/generator", -1, self->x, self->y, 0);

			setEntityAnimationByID(self, self->health);
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
		setEntityAnimationByID(self, self->health);
	}

	else if (self->active == FALSE && self->health < 0)
	{
		setEntityAnimationByID(self, -self->health);

		self->frameSpeed = 0;
	}

	self->action = &entityWait;
}
