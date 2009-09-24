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
#include "../inventory.h"
#include "../entity.h"
#include "../hud.h"
#include "../system/properties.h"
#include "../audio/audio.h"
#include "../collisions.h"

extern Entity *self, entity[MAX_ENTITIES];
extern Game game;

static void activate(int);
static void call(int);
static void reset(int);
static void wait(void);
static void wait(void);
static void initialise(void);
static void touch(Entity *);
static void resetTouch(Entity *);

Entity *addSwitch(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Switch\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->action = &initialise;

	if (strcmpignorecase(name, "common/call_switch") == 0)
	{
		e->activate = &call;
		e->touch = &touch;
	}

	else if (strcmpignorecase(name, "common/reset_switch") == 0)
	{
		e->activate = &reset;
		e->touch = &resetTouch;
	}

	else
	{
		e->activate = &activate;
		e->touch = &touch;
	}

	e->type = SWITCH;

	setEntityAnimation(e, STAND);

	return e;
}

static void call(int val)
{
	if (self->thinkTime == 0)
	{
		if (strlen(self->requires) != 0)
		{
			printf("Requires %s\n", self->requires);

			if (removeInventoryItem(self->requires) == 1)
			{
				setInfoBoxMessage(60,  _("Used %s"), self->requires);

				self->requires[0] = '\0';
			}

			else
			{
				setInfoBoxMessage(60,  _("%s is needed to activate this switch"), self->requires);

				return;
			}
		}

		playSoundToMap("sound/common/switch.ogg", -1, self->x, self->y, 0);

		self->active = TRUE;

		setEntityAnimation(self, WALK);

		self->thinkTime = 120;

		activateEntitiesValueWithObjectiveName(self->objectiveName, self->health);
	}
}

static void activate(int val)
{
	int remaining;

	if (strlen(self->requires) != 0)
	{
		if (removeInventoryItem(self->requires) == 1)
		{
			setInfoBoxMessage(60,  _("Used %s"), self->requires);

			self->requires[0] = '\0';
		}

		else
		{
			setInfoBoxMessage(60,  _("%s is needed to activate this switch"), self->requires);

			return;
		}
	}

	playSoundToMap("sound/common/switch.ogg", -1, self->x, self->y, 0);

	self->active = self->active == TRUE ? FALSE : TRUE;

	setEntityAnimation(self, self->active == TRUE ? WALK : STAND);

	if (self->active == TRUE)
	{
		remaining = countSiblings(self);

		if (remaining == 0)
		{
			activateEntitiesWithRequiredName(self->objectiveName, TRUE);
		}

		else
		{
			setInfoBoxMessage(30,  _("%d more to go..."), remaining);
		}
	}

	else
	{
		activateEntitiesWithRequiredName(self->objectiveName, FALSE);
	}

	if (self->maxThinkTime != 0)
	{
		/* Cancel the current timer */

		if (self->thinkTime > 0)
		{
			self->thinkTime = 0;

			stopSound(self->startX);
		}

		else
		{
			self->thinkTime = self->maxThinkTime;

			self->startX = playSoundToMap("sound/common/tick.ogg", -1, self->x, self->y, -1);
		}
	}
}

static void wait()
{
	if (self->thinkTime > 0 && strlen(self->requires) == 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			self->active = self->active == TRUE ? FALSE : TRUE;

			/* Switch time out */

			if (strcmpignorecase(self->name, "common/switch") == 0)
			{
				activateEntitiesWithRequiredName(self->objectiveName, self->active);
			}

			playSoundToMap("sound/common/switch.ogg", -1, self->x, self->y, 0);

			setEntityAnimation(self, self->active == TRUE ? WALK : STAND);

			if (self->maxThinkTime != 0)
			{
				stopSound(self->startX);
			}
		}
	}

	checkToMap(self);
}

static void reset(int val)
{
	Entity *e, *temp;

	if (self->thinkTime == 0)
	{
		playSoundToMap("sound/common/switch.ogg", -1, self->x, self->y, 0);

		self->active = TRUE;

		setEntityAnimation(self, WALK);

		self->thinkTime = 120;

		e = getEntityByObjectiveName(self->objectiveName);

		if (e != NULL && e->fallout != NULL)
		{
			temp = self;

			self = e;

			self->fallout();

			self->thinkTime = 0;

			self = temp;
		}
	}
}

static void initialise()
{
	setEntityAnimation(self, self->active == TRUE ? WALK : STAND);

	self->action = &wait;
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && game.showHints == TRUE)
	{
		setInfoBoxMessage(0,  _("Press Action to use this switch"));
	}
}

static void resetTouch(Entity *other)
{
	if (other->type == PLAYER && game.showHints == TRUE)
	{
		setInfoBoxMessage(0,  _("Press Action to reset puzzle blocks"));
	}
}
