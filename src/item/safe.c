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
#include "../entity.h"
#include "../event/script.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../inventory.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;
extern Input input;

static void init(void);
static void touch(Entity *);
static void entityWait(void);
static void activate(int);
static void readInputCode(void);
static void addDisplay(void);

Entity *addSafe(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Safe");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "CLOSED");

	return e;
}

static void init()
{
	if (self->active == TRUE)
	{
		self->touch = &touch;

		self->activate = &activate;
	}

	else
	{
		if (self->health == -99)
		{
			setEntityAnimation(self, "OPEN");
		}

		self->touch = NULL;

		self->activate = NULL;
	}

	self->action = &entityWait;
}

static void entityWait()
{

}

static void touch(Entity *other)
{
	setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
}

static void activate(int val)
{
	Entity *e;

	self->mental = 0;

	self->health = 0;

	if (strlen(self->requires) == 0)
	{
		e = getInventoryItemByObjectiveName("Scrap of Paper");

		if (e == NULL)
		{
			runScript("no_combination");

			return;
		}

		STRNCPY(self->requires, e->requires, sizeof(self->requires));

		runScript("has_combination");

		return;
	}

	addDisplay();

	self->action = &readInputCode;

	self->touch = NULL;

	self->activate = NULL;

	setPlayerLocked(TRUE);
}

static void readInputCode()
{
	int val, frameCount;
	char code[MAX_VALUE_LENGTH];

	if (input.left == 1)
	{
		input.left = 0;

		val = -1;

		playSound("sound/common/click");
	}

	else if (input.right == 1)
	{
		input.right = 0;

		val = 1;

		playSound("sound/common/click");
	}

	else if (input.attack == 1 || input.block == 1)
	{
		input.attack = 0;
		input.block = 0;

		val = 5;
	}

	else
	{
		val = 0;
	}

	if (val != 0)
	{
		if (val == 1 || val == -1)
		{
			if (self->health != val)
			{
				if (self->health != 0)
				{
					snprintf(code, sizeof(code), "%s%d%s", self->target->objectiveName, abs(self->mental), self->health == -1 ? "L" : "R");

					STRNCPY(self->target->objectiveName, code, sizeof(self->target->objectiveName));

					self->mental = 0;
				}

				self->health = val;
			}

			self->target->currentFrame += val;

			frameCount = getFrameCount(self->target);

			if (self->target->currentFrame >= frameCount)
			{
				self->target->currentFrame = 0;
			}

			else if (self->target->currentFrame < 0)
			{
				self->target->currentFrame = frameCount - 1;
			}

			self->mental += val;
		}

		if (val == 5)
		{
			snprintf(code, sizeof(code), "%s%d%s", self->target->objectiveName, abs(self->mental), self->health == -1 ? "L" : "R");

			STRNCPY(self->target->objectiveName, code, sizeof(self->target->objectiveName));

			if (strcmpignorecase(self->target->objectiveName, self->requires) == 0)
			{
				self->target->inUse = FALSE;

				self->action = &entityWait;

				setPlayerLocked(FALSE);

				runScript("correct_combination");
			}

			else
			{
				self->target->inUse = FALSE;

				self->action = &entityWait;

				setPlayerLocked(FALSE);

				self->activate = &activate;

				self->touch = &touch;

				runScript("incorrect_combination");
			}
		}
	}
}

static void addDisplay()
{
	Entity *e;

	e = getFreeEntity();

	loadProperties("item/safe_display", e);

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;

	e->draw = &drawLoopingAnimationToMap;

	e->frameSpeed = 0;

	setEntityAnimation(e, "STAND");

	e->x = self->x + self->w / 2 - e->w / 2;
	e->y = self->y - 64;

	self->target = e;
}
