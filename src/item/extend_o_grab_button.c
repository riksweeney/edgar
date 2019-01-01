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
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void init(void);
static void entityWait(void);
static void touch(Entity *);

Entity *addExtendOGrabButton(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Extend-O-Grab Button");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->thinkTime = 0;
	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &touch;
	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	setEntityAnimation(self, self->active == FALSE ? "STAND" : "WALK");

	self->dirY = 0;

	self->action = &entityWait;
}

static void entityWait()
{
	if (self->mental > 0)
	{
		self->mental--;
	}

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->thinkTime = 0;

			self->active = self->active == TRUE ? FALSE : TRUE;

			setEntityAnimation(self, self->active == FALSE ? "STAND" : "WALK");

			activateEntitiesWithRequiredName(self->objectiveName, self->active);

			playSoundToMap("sound/common/switch", -1, self->x, self->y, 0);

			stopSound(self->endX);
		}
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	int total, remaining;

	if (self->mental == 0)
	{
		if (strcmpignorecase(other->name, "item/extend_o_grab") == 0)
		{
			/* Don't toggle if a timer */

			if (self->maxThinkTime != 0 && self->thinkTime != 0)
			{
				self->thinkTime = self->maxThinkTime;

				return;
			}

			total = 0;

			remaining = self->maxThinkTime == 0 ? countSiblings(self, &total) : 0;

			if (remaining == 0)
			{
				self->active = self->active == TRUE ? FALSE : TRUE;

				setEntityAnimation(self, self->active == FALSE ? "STAND" : "WALK");

				activateEntitiesWithRequiredName(self->objectiveName, self->active);

				self->thinkTime = self->maxThinkTime;

				if (self->maxThinkTime != 0)
				{
					self->endX = playSoundToMap("sound/common/tick", -1, self->x, self->y, -1);
				}

				else if (total > 0)
				{
					setInfoBoxMessage(30, 255, 255, 255, _("Complete"), remaining);
				}
			}

			else
			{
				setInfoBoxMessage(30, 255, 255, 255, _("%d more to go..."), remaining);
			}

			playSoundToMap("sound/common/switch", -1, self->x, self->y, 0);

			self->mental = 30;
		}

		else
		{
			pushEntity(other);
		}
	}
}
