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
#include "../graphics/animation.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void entityWait(void);
static void init(void);
static void touch(Entity *);
static void callTouch(Entity *);

Entity *addArrowTarget(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Arrow Target");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;

	e->type = KEY_ITEM;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	if (self->mental == -1)
	{
		if (self->dirY == 0)
		{
			self->dirY = self->speed;
		}

		self->y += self->dirY;

		if (self->y >= self->endY)
		{
			self->y = self->endY;

			self->dirY = -self->dirY;
		}

		else if (self->y <= self->startY)
		{
			self->y = self->startY;

			self->dirY = -self->dirY;
		}
	}

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->thinkTime = 0;

			self->active = self->active == TRUE ? FALSE : TRUE;

			setEntityAnimation(self, self->active == FALSE ? "STAND" : "WALK");

			if (self->mental != 1)
			{
				activateEntitiesWithRequiredName(self->objectiveName, self->active);
			}

			playSoundToMap("sound/common/switch", -1, self->x, self->y, 0);

			stopSound(self->endX);
		}
	}
}

static void init()
{
	setEntityAnimation(self, self->active == FALSE ? "STAND" : "WALK");

	self->action = &entityWait;

	self->touch = self->mental == 1 ? &callTouch : &touch;
}

static void touch(Entity *other)
{
	int remaining, total;
	Entity *temp;

	if (strcmpignorecase(other->name, self->requires) == 0)
	{
		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		/* Don't toggle if a timer */

		if (self->maxThinkTime != 0 && self->thinkTime != 0)
		{
			self->thinkTime = self->maxThinkTime;

			return;
		}

		total = 0;

		remaining = self->maxThinkTime == 0 ? countSiblings(self, &total) : 0;

		self->active = self->active == TRUE ? FALSE : TRUE;

		if (remaining == 0)
		{
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
			setEntityAnimation(self, self->active == FALSE ? "STAND" : "WALK");

			setInfoBoxMessage(30, 255, 255, 255, _("%d more to go..."), remaining);
		}

		playSoundToMap("sound/common/switch", -1, self->x, self->y, 0);
	}
}

static void callTouch(Entity *other)
{
	if (self->thinkTime == 0)
	{
		if (strcmpignorecase(other->name, self->requires) == 0)
		{
			playSoundToMap("sound/common/switch", -1, self->x, self->y, 0);

			self->active = TRUE;

			setEntityAnimation(self, "WALK");

			self->thinkTime = 120;

			if (strlen(self->objectiveName) != 0)
			{
				activateEntitiesValueWithObjectiveName(self->objectiveName, self->health);
			}
		}
	}
}
