/*
Copyright (C) 2009 Parallel Realities

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
#include "bubble.h"
#include "../system/error.h"
#include "../system/random.h"
#include "../player.h"
#include "../hud.h"
#include "../inventory.h"

extern Entity *self, player;
extern Input input;

static void activate(int);
static void wait(void);
static void init(void);
static void touch(Entity *);
static void displayInputCode(void);
static void generateInputCode(void);
static void readInputCode(void);
static void clearInputCode(void);

Entity *addCodeDoor(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Code Door");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;
	e->touch = &touch;
	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self->target, 0);
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && self->active == FALSE)
	{
		setInfoBoxMessage(0, _("Press Action to interact"));
	}
}

static void activate(int val)
{
	if (strlen(self->requires) != 0)
	{
		if (removeInventoryItem(self->requires) == TRUE)
		{
			self->requires[0] = '\0';
		}

		else
		{
			setInfoBoxMessage(60, _("%s is required"), _(self->requires));

			return;
		}
	}

	generateInputCode();

	self->mental = 0;

	self->thinkTime = 120;

	setInfoBoxMessage(120, _("Repeat the sequence"));

	self->action = &displayInputCode;

	self->touch = NULL;

	self->activate = NULL;

	setPlayerLocked(TRUE);
}

static void generateInputCode()
{
	int i, generate, dir;

	generate = FALSE;

	/* Only generate code once */

	if (self->health > 0)
	{
		if (strlen(self->target->requires) == 0)
		{
			generate = TRUE;
		}
	}

	/* Always generate code */

	else
	{
		generate = TRUE;
	}

	if (generate == TRUE)
	{
		for (i=0;i<abs(self->health);i++)
		{
			dir = (prand() % 4) + 1;

			self->target->requires[i] = dir;
		}

		self->target->requires[i] = '\0';
	}
}

static void displayInputCode()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = self->maxThinkTime;

		self->target->health = (int)self->target->requires[self->mental];

		setEntityAnimation(self->target, self->target->health);

		if (self->mental == strlen(self->target->requires))
		{
			self->mental = 0;

			self->action = &readInputCode;
		}

		else
		{
			playSoundToMap("sound/item/charge_beep.ogg", -1, self->x, self->y, 0);

			self->mental++;

			self->action = &clearInputCode;
		}
	}
}

static void clearInputCode()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self->target, 0);

		self->thinkTime = self->mental > strlen(self->target->requires) ? 0 : self->maxThinkTime;

		self->action = &displayInputCode;
	}
}

static void readInputCode()
{
	int val;

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

	else
	{
		val = -1;
	}

	if (val != -1)
	{
		setEntityAnimation(self->target, val);

		if ((int)self->target->requires[self->mental] == val)
		{
			playSoundToMap("sound/item/charge_beep.ogg", -1, self->x, self->y, 0);

			self->mental++;

			if (self->target->requires[self->mental] == '\0')
			{
				setPlayerLocked(FALSE);

				activateEntitiesWithRequiredName(self->objectiveName, TRUE);

				setInfoBoxMessage(60, _("Correct Sequence"));

				self->active = TRUE;

				self->touch = NULL;

				self->action = &wait;
			}
		}

		else
		{
			playSoundToMap("sound/item/buzzer.ogg", -1, self->x, self->y, 0);

			self->mental = 0;

			self->action = &wait;

			self->thinkTime = self->maxThinkTime;

			self->activate = &activate;

			self->touch = &touch;

			setInfoBoxMessage(60, _("Incorrect Sequence"));

			setPlayerLocked(FALSE);
		}

		self->thinkTime = self->maxThinkTime;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;

		setEntityAnimation(self->target, 0);
	}
}

static void init()
{
	char display[MAX_VALUE_LENGTH];
	Entity *e;

	if (self->active == TRUE)
	{
		self->touch = NULL;
	}

	else
	{
		setEntityAnimation(self, STAND);

		snprintf(display, MAX_VALUE_LENGTH, "%s_DISPLAY", self->objectiveName);

		e = getEntityByObjectiveName(display);

		if (e == NULL)
		{
			showErrorAndExit("Code Door could not find display %s", display);
		}

		self->target = e;
	}

	self->action = &wait;
}
