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
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;
extern Game game;

static void activate(int);
static void call(int);
static void reset(int);
static void entityWait(void);
static void initialise(void);
static void touch(Entity *);
static void resetTouch(Entity *);

Entity *addSwitch(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Switch");
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

	setEntityAnimation(e, "STAND");

	return e;
}

static void call(int val)
{
	if (val != 0)
	{
		return;
	}

	if (self->thinkTime == 0)
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
				setInfoBoxMessage(60, 255, 255, 255, _("%s is needed to activate this switch"), _(self->requires));

				return;
			}
		}

		playSoundToMap("sound/common/switch", -1, self->x, self->y, 0);

		self->active = TRUE;

		setEntityAnimation(self, "WALK");

		self->thinkTime = 120;

		if (strlen(self->objectiveName) != 0)
		{
			if (self->mental == -1)
			{
				activateEntitiesWithRequiredName(self->objectiveName, TRUE);
			}

			else
			{
				activateEntitiesValueWithObjectiveName(self->objectiveName, self->health);
			}
		}

		else
		{
			printf("Switch at %d %d has no Target\n", (int)self->x, (int)self->y);
		}
	}
}

static void activate(int val)
{
	int remaining, total;

	if (strlen(self->requires) != 0)
	{
		if (removeInventoryItemByObjectiveName(self->requires) == TRUE)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("Used %s"), _(self->requires));

			self->requires[0] = '\0';
		}

		else
		{
			setInfoBoxMessage(60, 255, 255, 255, _("%s is needed to activate this switch"), _(self->requires));

			return;
		}
	}

	playSoundToMap("sound/common/switch", -1, self->x, self->y, 0);

	self->active = self->active == TRUE ? FALSE : TRUE;

	setEntityAnimation(self, self->active == TRUE ? "WALK" : "STAND");

	if (self->active == TRUE)
	{
		total = 0;

		remaining = self->maxThinkTime == 0 ? countSiblings(self, &total) : 0;

		if (remaining == 0)
		{
			activateEntitiesWithRequiredName(self->objectiveName, TRUE);

			if (self->mental == -1)
			{
				STRNCPY(self->objectiveName, "-", sizeof(self->objectiveName));
			}

			if (total > 0 && self->maxThinkTime == 0)
			{
				setInfoBoxMessage(30, 255, 255, 255, _("Complete"));
			}
		}

		else
		{
			setInfoBoxMessage(30, 255, 255, 255, _("%d more to go..."), remaining);
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

			stopSound(self->endX);
		}

		else
		{
			self->thinkTime = self->maxThinkTime;

			self->endX = playSoundToMap("sound/common/tick", -1, self->x, self->y, -1);
		}
	}
}

static void entityWait()
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

			playSoundToMap("sound/common/switch", -1, self->x, self->y, 0);

			setEntityAnimation(self, self->active == TRUE ? "WALK" : "STAND");

			if (self->maxThinkTime != 0)
			{
				stopSound(self->endX);
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
		playSoundToMap("sound/common/switch", -1, self->x, self->y, 0);

		self->active = TRUE;

		setEntityAnimation(self, "WALK");

		self->thinkTime = 120;

		e = self->health < 0 ? getEntityByRequiredName(self->objectiveName) : getEntityByObjectiveName(self->objectiveName);

		if (e != NULL && e->fallout != NULL)
		{
			temp = self;

			self = e;

			self->fallout();

			self->thinkTime = 0;

			self = temp;
		}

		else
		{
			printf("Couldn't reset %s\n", self->objectiveName);

			if (e == NULL)
			{
				printf("No item found\n");
			}

			else if (e->fallout == NULL)
			{
				printf("No fallout found\n");
			}
		}
	}
}

static void initialise()
{
	setEntityAnimation(self, self->active == TRUE ? "WALK" : "STAND");

	#if DEV == 1
	if (strlen(self->objectiveName) == 0)
	{
		showErrorAndExit("Switch at %d %d has no Target", (int)self->x, (int)self->y);
	}
	#endif

	self->action = &entityWait;
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && game.showHints == TRUE)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to use this switch"));
	}
}

static void resetTouch(Entity *other)
{
	if (other->type == PLAYER && game.showHints == TRUE)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to reset puzzle blocks"));
	}
}
