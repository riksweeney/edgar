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
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self, player;
extern Input input;

static void activate(int);
static void entityWait(void);
static void init(void);
static void touch(Entity *);
static void readInputCode(void);

Entity *addInstructionMachine(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Instruction Machine");
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
	Entity *e = getInventoryItemByObjectiveName(self->requires);

	if (e == NULL)
	{
		setInfoBoxMessage(60, 255, 255, 255, _("%s is required"), _(self->requires));
	}

	else
	{
		self->mental = 0;

		setInfoBoxMessage(300, 255, 255, 255, _("Enter the directions"));

		self->target->requires[0] = '\0';

		self->action = &readInputCode;

		self->touch = NULL;

		self->activate = NULL;

		setPlayerLocked(TRUE);
	}
}

static void readInputCode()
{
	char c;
	int val;
	Entity *e;

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

	if (val == 5 || self->mental > 30)
	{
		e = getInventoryItemByObjectiveName(self->requires);

		setPlayerLocked(FALSE);

		STRNCPY(e->requires, self->target->requires, sizeof(self->target->requires));

		self->action = &entityWait;

		if (self->mental > 30)
		{
			setInfoBoxMessage(300, 255, 255, 255, _("Out of space for instructions"));
		}

		self->touch = &touch;
		self->activate = &activate;

		self->action = &entityWait;
	}

	else if (val != -1)
	{
		setEntityAnimationByID(self->target, val);

		playSoundToMap("sound/item/charge_beep", -1, self->x, self->y, 0);

		switch (val)
		{
			case 1:
				c = 'u';
			break;

			case 2:
				c = 'd';
			break;

			case 3:
				c = 'l';
			break;

			default:
				c = 'r';
			break;
		}

		self->target->requires[self->mental] = c;

		self->target->requires[self->mental + 1] = '\0';

		self->mental++;

		self->thinkTime = self->maxThinkTime;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;

		setEntityAnimationByID(self->target, 0);
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
		showErrorAndExit("Instruction Machine could not find display %s", display);
	}

	self->target = e;

	self->touch = &touch;
	self->activate = &activate;

	self->action = &entityWait;
}
