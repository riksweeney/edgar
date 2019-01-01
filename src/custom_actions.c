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

#include "headers.h"

#include "custom_actions.h"
#include "entity.h"
#include "hud.h"
#include "medal.h"
#include "player.h"
#include "system/error.h"

extern Entity *self;
extern Game game;

void setCustomAction(Entity *e, void (*func)(int *, int *, float *), int thinkTime, int accumulates, float customValue)
{
	int i;
	Entity *temp;

	for (i=0;i<MAX_CUSTOM_ACTIONS;i++)
	{
		/* Search for an already existing action */

		if (e->customAction[i].thinkTime != 0 && e->customAction[i].action == func)
		{
			if (thinkTime >= e->customAction[i].thinkTime)
			{
				e->customAction[i].thinkTime = thinkTime;

				e->customAction[i].counter += accumulates;

				e->customAction[i].value = customValue;
			}

			return;
		}
	}

	for (i=0;i<MAX_CUSTOM_ACTIONS;i++)
	{
		if (e->customAction[i].thinkTime == 0)
		{
			e->customAction[i].action = func;

			e->customAction[i].thinkTime = thinkTime + 1;

			e->customAction[i].counter = accumulates;

			e->customAction[i].value = customValue;

			/* Execute the custom action once */

			temp = self;

			self = e;

			self->customAction[i].action(&self->customAction[i].thinkTime, &self->customAction[i].counter, &self->customAction[i].value);

			self = temp;

			return;
		}
	}

	showErrorAndExit("No free slots for Custom Action");
}

void clearCustomAction(Entity *e, void (*func)(int *, int *, float *))
{
	int i;

	for (i=0;i<MAX_CUSTOM_ACTIONS;i++)
	{
		/* Search for an already existing action */

		if (e->customAction[i].thinkTime != 0 && e->customAction[i].action == func)
		{
			e->customAction[i].thinkTime = 0;

			return;
		}
	}
}

int hasCustomAction(Entity *e, void (*func)(int *, int *, float *))
{
	int i;

	for (i=0;i<MAX_CUSTOM_ACTIONS;i++)
	{
		/* Search for an already existing action */

		if (e->customAction[i].thinkTime != 0 && e->customAction[i].action == func)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void clearCustomActions(Entity *e)
{
	int i;

	for (i=0;i<MAX_CUSTOM_ACTIONS;i++)
	{
		e->customAction[i].thinkTime = 0;
	}
}

void addCustomActionFromScript(Entity *e, char *line)
{
	char actionName[MAX_VALUE_LENGTH];
	int thinkTime, accumulates;
	void (*action)(int *, int *, float *);

	action = NULL;

	sscanf(line, "%s %d %d", actionName, &thinkTime, &accumulates);

	if (strcmpignorecase(actionName, "REGENERATE") == 0)
	{
		action = &regenerate;
	}

	else if (strcmpignorecase(actionName, "BECOME_SLIME") == 0)
	{
		action = &slimeTimeout;
	}

	if (action != NULL)
	{
		setCustomAction(e, action, thinkTime, accumulates, 0);
	}
}

void doCustomAction(CustomAction *customAction)
{
	customAction->action(&customAction->thinkTime, &customAction->counter, &customAction->value);
}

void helpless(int *thinkTime, int *counter, float *value)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->flags |= HELPLESS;
	}

	else
	{
		if (!(self->flags & ATTRACTED) && !(self->flags & FRICTIONLESS))
		{
			self->dirX = 0;
		}

		if (self->flags & FLY)
		{
			self->dirY = 0;
		}
	}
}

void invulnerable(int *thinkTime, int *counter, float *value)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->flags |= INVULNERABLE;
	}

	if ((*thinkTime) % 3 == 0)
	{
		self->flags ^= NO_DRAW;
	}

	if ((*thinkTime) == 0)
	{
		self->flags &= ~NO_DRAW;
	}
}

void invulnerableNoFlash(int *thinkTime, int *counter, float *value)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->flags |= INVULNERABLE;
	}
}

void flashWhite(int *thinkTime, int *counter, float *value)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->flags |= FLASH;
	}
}

void slowDown(int *thinkTime, int *counter, float *value)
{
	(*thinkTime)--;

	if (*counter <= 0)
	{
		(*counter) = 1;
	}

	if (*thinkTime != 0)
	{
		self->frameSpeed = 1.0f / (*counter);
		self->speed = self->originalSpeed / (*counter);
	}

	else
	{
		self->frameSpeed = 1;
		self->speed = self->originalSpeed;
	}
}

void dizzy(int *thinkTime, int *counter, float *value)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->flags |= HELPLESS;
	}
}

void regenerate(int *thinkTime, int *counter, float *value)
{
	self->health = self->maxHealth;
}

void slimeTimeout(int *thinkTime, int *counter, float *value)
{
	int timeBefore, timeAfter;

	game.timeSpentAsSlime++;

	if (game.timeSpentAsSlime == 36000)
	{
		addMedal("slime");
	}

	timeBefore = *thinkTime / 60;

	(*thinkTime)--;

	timeAfter = *thinkTime / 60;

	if (timeBefore != timeAfter)
	{
		setSlimeTimerValue(timeAfter);
	}

	if (*thinkTime == 0)
	{
		becomeEdgar();
	}
}

void invisible(int *thinkTime, int *counter, float *value)
{
	(*thinkTime)--;

	self->alpha = *thinkTime == 0 ? 255 : 64;
}

void attract(int *thinkTime, int *counter, float *value)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->flags |= ATTRACTED;

		self->dirX = *value;
	}
}

void antiGravity(int *thinkTime, int *counter, float *value)
{
	(*thinkTime)--;

	if (*counter <= 0)
	{
		(*counter) = 1;
	}

	if (!(self->flags & FLY))
	{
		if (*thinkTime != 0)
		{
			self->weight = 0;
			self->dirY = -1 * (*counter);
		}

		else
		{
			self->weight = self->originalWeight;
			self->dirY = 0;
		}
	}
}

void spriteTrail(int *thinkTime, int *counter, float *value)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		(*counter)--;

		if (*counter <= 0)
		{
			*counter = 5;

			addDuplicateImage(self);
		}
	}
}

void stickToFloor(int *thinkTime, int *counter, float *value)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->speed = self->originalSpeed * 0.25;

		self->flags |= GROUNDED;
	}

	else
	{
		self->speed = self->originalSpeed;

		self->flags &= ~GROUNDED;
	}
}

void removeFriction(int *thinkTime, int *counter, float *value)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->flags |= FRICTIONLESS;
	}
}

void confused(int *thinkTime, int *counter, float *value)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->flags |= CONFUSED;
	}
}
