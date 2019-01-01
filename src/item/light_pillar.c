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
#include "../inventory.h"
#include "../map.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void entityWait(void);
static void addLight(int);
static void touch(Entity *);
static void init(void);

Entity *addLightPillar(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Light Pillar");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &touch;
	e->activate = &addLight;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	setEntityAnimation(self, self->thinkTime > 0 ? "WALK" : "STAND");

	self->action = &entityWait;
}

static void entityWait()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			setDarkMap(TRUE);

			self->active = FALSE;

			stopSound(self->endX);

			setEntityAnimation(self, "STAND");
		}

		else
		{
			setDarkMap(FALSE);
		}
	}

	checkToMap(self);
}

static void addLight(int val)
{
	if (strlen(self->requires) != 0)
	{
		if (removeInventoryItemByObjectiveName(self->requires) == TRUE)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("Used %s"), _(self->requires));

			self->requires[0] = '\0';
		}

		else
		{
			setInfoBoxMessage(60, 255, 255, 255, _("%s is needed to activate this Light Pillar"), _(self->requires));

			return;
		}
	}

	if (self->active == TRUE)
	{
		self->thinkTime = 0;

		self->active = FALSE;

		stopSound(self->endX);

		setDarkMap(TRUE);

		setEntityAnimation(self, "STAND");
	}

	else
	{
		self->thinkTime = self->maxThinkTime;

		self->endX = playSoundToMap("sound/common/tick", -1, self->x, self->y, -1);

		self->active = TRUE;

		setDarkMap(FALSE);

		setEntityAnimation(self, "WALK");
	}
}

static void touch(Entity *other)
{
	setInfoBoxMessage(0, 255, 255, 255, _("Press Action to activate the Light Pillar"));
}
