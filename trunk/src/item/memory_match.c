/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "../collisions.h"
#include "../system/error.h"
#include "../system/random.h"
#include "../player.h"
#include "../hud.h"
#include "../map.h"
#include "../event/script.h"
#include "../inventory.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"

extern Entity *self;
extern Input input;

static void activate(int);
static void entityWait(void);
static void init(void);
static void touch(Entity *);
static void readInputCode(void);

Entity *addMemoryMatch(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Memory Match");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void entityWait()
{
	setEntityAnimation(self, self->active == TRUE ? WALK : STAND);

	checkToMap(self);
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && self->active == TRUE)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
	}
}

static void activate(int val)
{
	if (self->active == TRUE)
	{
		self->target->mental = -1;

		self->action = &readInputCode;

		self->touch = NULL;

		self->activate = NULL;

		setPlayerLocked(TRUE);
	}
}

static void readInputCode()
{
	int val;
	Entity *temp;

	if (self->target->mental == 2)
	{
		setPlayerLocked(FALSE);

		setInfoBoxMessage(60, 255, 255, 255, _("Complete"));

		self->action = &entityWait;

		self->touch = NULL;
		self->activate = NULL;

		fireTrigger(self->objectiveName);

		fireGlobalTrigger(self->objectiveName);

		return;
	}

	else if (self->target->mental == 1)
	{
		resetCameraLimits();

		setInfoBoxMessage(60, 255, 255, 255, _("Failed"));

		setPlayerLocked(FALSE);

		self->action = &entityWait;

		self->touch = &touch;

		self->activate = &activate;

		return;
	}

	if (input.left == 1)
	{
		input.left = 0;

		val = 1;
	}

	else if (input.right == 1)
	{
		input.right = 0;

		val = 2;
	}

	else if (input.up == 1)
	{
		input.up = 0;

		val = 3;
	}

	else if (input.down == 1)
	{
		input.down = 0;

		val = 4;
	}

	else if (input.attack == 1)
	{
		input.attack = 0;

		val = 5;
	}

	else if (input.interact == 1)
	{
		input.interact = 0;

		val = 6;
	}

	else
	{
		val = -1;
	}

	if (val == 6)
	{
		setPlayerLocked(FALSE);

		self->target->mental = 1;

		self->action = &entityWait;

		self->touch = &touch;
		self->activate = &activate;
	}

	else if (val != -1 && self->target->activate != NULL)
	{
		temp = self;

		self = self->target;

		self->activate(val);

		self = temp;
	}
}

static void init()
{
	char display[MAX_VALUE_LENGTH];
	Entity *e;

	setEntityAnimation(self, STAND);

	snprintf(display, MAX_VALUE_LENGTH, "%s_DISPLAY", self->objectiveName);

	e = getEntityByObjectiveName(display);

	if (e == NULL)
	{
		showErrorAndExit("Memory Match could not find display %s", display);
	}

	self->target = e;

	if (self->target->mental != 2)
	{
		self->touch = &touch;
		self->activate = &activate;
	}

	self->action = &entityWait;
}
