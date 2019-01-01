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
extern Game game;

static void init(void);
static void entityWait(void);
static void activate(int);
static void touch(Entity *);

Entity *addPressurePlate(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Pressure Plate");
	}

	loadProperties(name, e);

	e->touch = &touch;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;

	e->activate = &activate;

	e->x = x;
	e->y = y;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	setEntityAnimation(self, self->thinkTime <= 0 ? "STAND" : "WALK");

	self->action = &entityWait;
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		setEntityAnimation(self, "STAND");

		activateEntitiesWithRequiredName(self->objectiveName, FALSE);

		self->active = FALSE;
	}

	else if (self->thinkTime < 0)
	{
		self->thinkTime = -1;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	/* Place the entity as close as possible */

	other->y = self->y + self->box.y;
	other->y -= other->box.h + other->box.y;

	other->standingOn = self;
	other->dirY = 0;
	other->flags |= ON_GROUND;

	if (other->standingOn == self)
	{
		if (strlen(self->requires) == 0 || strcmpignorecase(self->requires, other->objectiveName) == 0)
		{
			activate(1);
		}

		else if (other->type == PLAYER)
		{
			setInfoBoxMessage(0, 255, 255, 255, _("%s is required to use this Pressure Plate"), _(self->requires));
		}
	}
}

static void activate(int val)
{
	int remaining, total;

	if (self->thinkTime == -1)
	{
		setEntityAnimation(self, "WALK");

		self->active = TRUE;

		remaining = self->mental == -1 ? 0 : countSiblings(self, &total);

		if (remaining == 0)
		{
			activateEntitiesWithRequiredName(self->objectiveName, TRUE);

			if (total > 0 && self->mental != -1)
			{
				setInfoBoxMessage(30, 255, 255, 255, _("Complete"), remaining);
			}
		}

		else
		{
			setInfoBoxMessage(30, 255, 255, 255, _("%d more to go..."), remaining);
		}

		self->maxHealth = self->health;

		playSoundToMap("sound/common/switch", -1, self->x, self->y, 0);
	}

	self->thinkTime = 5;
}
