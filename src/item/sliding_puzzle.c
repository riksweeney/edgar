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
#include "../graphics/animation.h"
#include "../hud.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;
extern Input input;

static void activate(int);
static void entityWait(void);
static void init(void);
static void touch(Entity *);
static void readInputCode(void);

Entity *addSlidingPuzzle(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Sliding Puzzle");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	checkToMap(self);
}

static void touch(Entity *other)
{
	if (other->type == PLAYER)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
	}
}

static void activate(int val)
{
	self->mental = 0;

	setInfoBoxMessage(300, 255, 255, 255, _("Solve the sliding puzzle"));

	self->target->requires[0] = '\0';

	self->action = &readInputCode;

	self->touch = NULL;

	self->activate = NULL;

	setPlayerLocked(TRUE);
}

static void readInputCode()
{
	int val;
	Entity *temp;

	if (self->target->mental == 1)
	{
		setPlayerLocked(FALSE);

		setInfoBoxMessage(60, 255, 255, 255, _("Complete"));

		self->action = &entityWait;

		self->touch = NULL;
		self->activate = NULL;

		activateEntitiesWithRequiredName(self->objectiveName, TRUE);

		return;
	}

	if (input.up == 1)
	{
		input.up = 0;

		val = 1;
	}

	else if (input.down == 1)
	{
		input.down = 0;

		val = 2;
	}

	else if (input.left == 1)
	{
		input.left = 0;

		val = 3;
	}

	else if (input.right == 1)
	{
		input.right = 0;

		val = 4;
	}

	else if (input.attack == 1)
	{
		input.attack = 0;

		val = 5;
	}

	else
	{
		val = -1;
	}

	if (val == 5)
	{
		setPlayerLocked(FALSE);

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

	setEntityAnimation(self, "STAND");

	snprintf(display, MAX_VALUE_LENGTH, "%s_DISPLAY", self->objectiveName);

	e = getEntityByObjectiveName(display);

	if (e == NULL)
	{
		showErrorAndExit("Sliding Puzzle could not find display %s", display);
	}

	self->target = e;

	if (self->target->mental == 0)
	{
		self->touch = &touch;
		self->activate = &activate;
	}

	self->action = &entityWait;
}
