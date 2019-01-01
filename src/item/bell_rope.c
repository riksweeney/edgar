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
#include "../event/global_trigger.h"
#include "../event/trigger.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../inventory.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "key_items.h"

extern Entity *self;

static void init(void);
static void entityWait(void);
static void activate(int);
static void touch(Entity *);

Entity *addBellRope(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Bell Rope");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;

	e->touch = &touch;

	e->fallout = &keyItemFallout;

	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "NO_BELL");

	return e;
}

static void init()
{
	if (self->active == TRUE)
	{
		setEntityAnimation(self, "BELL");
	}

	if (self->mental == -1)
	{
		self->touch = NULL;

		self->flags |= NO_DRAW;
	}

	else
	{
		self->head = getEntityByObjectiveName(self->requires);

		if (self->head == NULL)
		{
			showErrorAndExit("Bell Rope cannot find %s", self->requires);
		}
	}

	self->action = &entityWait;
}

static void entityWait()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, "BELL");
		}
	}
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
	int i;
	char tune[MAX_VALUE_LENGTH];

	if (self->mental == -1)
	{
		if (self->active == FALSE && val == 100)
		{
			for (i=0;i<8;i++)
			{
				snprintf(tune, MAX_VALUE_LENGTH, "%ld", prand() % 5);

				self->requires[i] = tune[0];
			}

			self->requires[i] = '\0';

			self->active = TRUE;
		}
	}

	else
	{
		if (self->active == FALSE)
		{
			if (removeInventoryItemByObjectiveName("Bell") == TRUE)
			{
				setEntityAnimation(self, "BELL");

				self->active = TRUE;
			}

			else
			{
				setInfoBoxMessage(60, 255, 255, 255, _("%s is required"), _("Bell"));
			}
		}

		else
		{
			self->thinkTime = 90;

			setEntityAnimation(self, "RING");

			snprintf(tune, MAX_VALUE_LENGTH, "sound/item/bell%d", self->mental);

			playSoundToMap(tune, -1, self->x, self->y, 0);

			if (self->head->active == TRUE)
			{
				snprintf(tune, MAX_VALUE_LENGTH, "%d", self->mental);

				self->head->description[self->head->health] = tune[0];

				self->head->health++;

				self->head->description[self->head->health] = '\0';

				if (self->head->health + 1 >= MAX_VALUE_LENGTH)
				{
					for (i=1;i<MAX_VALUE_LENGTH;i++)
					{
						self->head->description[i - 1] = self->head->description[i];

						self->head->description[i] = '\0';
					}

					self->head->health = MAX_VALUE_LENGTH - 2;
				}

				if (strstr(self->head->description, self->head->requires) != NULL)
				{
					fireTrigger(self->head->objectiveName);

					fireGlobalTrigger(self->head->objectiveName);
				}
			}
		}
	}
}
