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

#include "headers.h"

#include "graphics/animation.h"
#include "player.h"
#include "map.h"
#include "hud.h"
#include "custom_actions.h"

extern Entity *self;

void setCustomAction(Entity *e, void (*func)(int *, int *), int thinkTime, int accumulates)
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

			/* Execute the custom action once */

			temp = self;

			self = e;

			self->customAction[i].action(&self->customAction[i].thinkTime, &self->customAction[i].counter);

			self = temp;

			return;
		}
	}

	printf("No free slots for Custom Action\n");

	exit(1);
}

void clearCustomAction(Entity *e, void (*func)(int *, int *))
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
	void (*action)(int *, int *);
	
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
		setCustomAction(e, action, thinkTime, accumulates);
	}
}

void doCustomAction(CustomAction *customAction)
{
	customAction->action(&customAction->thinkTime, &customAction->counter);
}

void helpless(int *thinkTime, int *counter)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->flags |= HELPLESS;
	}

	else
	{
		self->dirX = 0;

		if (self->flags & FLY)
		{
			self->dirY = 0;
		}
	}
}

void invulnerable(int *thinkTime, int *counter)
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

void invulnerableNoFlash(int *thinkTime, int *counter)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->flags |= INVULNERABLE;
	}
}

void flashWhite(int *thinkTime, int *counter)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->flags |= FLASH;
	}
}

void slowDown(int *thinkTime, int *counter)
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

void dizzy(int *thinkTime, int *counter)
{
	(*thinkTime)--;

	if (*thinkTime != 0)
	{
		self->flags |= HELPLESS;
	}
}

void regenerate(int *thinkTime, int *counter)
{
	self->health = self->maxHealth;
}

void slimeTimeout(int *thinkTime, int *counter)
{
	(*thinkTime)--;
	
	setInfoBoxMessage(5,  _("%d"), *thinkTime / 60);

	if (*thinkTime == 0)
	{
		becomeEdgar();
	}
}

void invisible(int *thinkTime, int *counter)
{
	(*thinkTime)--;
	
	self->alpha = *thinkTime == 0 ? 255 : 64;
}
