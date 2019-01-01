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
#include "../inventory.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void touch(Entity *);
static void entityWait(void);
static void setStart(void);
static void moveToTarget(void);

Entity *addDoor(char *name, int x, int y, int type)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Door");
	}

	loadProperties(strcmpignorecase(name, "common/wooden_door") == 0 ? "door/metal_door" : name, e);

	e->x = x;
	e->y = y;

	e->type = type;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->action = &setStart;

	e->flags |= OBSTACLE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void setStart()
{
	self->face = RIGHT;

	if ((self->endX == 0 && self->endY == 0) || (self->endX == self->startX && self->endY == self->startY))
	{
		printf("WARNING: Door %s at %0.0f %0.0f has no valid end\n", self->objectiveName, self->x, self->y);

		self->endY = self->startY - self->h;

		printf("Setting to %f\n", self->endY);
	}

	if (self->speed == 0)
	{
		showErrorAndExit("Door at %0.0f %0.0f has no speed and will not move", self->x, self->y);
	}

	if (self->type == MANUAL_DOOR)
	{
		self->targetX = self->endX;
		self->targetY = self->endY;

		self->action = &entityWait;
	}

	else
	{
		self->action = &moveToTarget;
	}
}

static void entityWait()
{
	self->dirX = self->dirY = 0;
}

static void touch(Entity *other)
{
	pushEntity(other);

	if (self->type == MANUAL_DOOR)
	{
		if (other->type == PLAYER && self->active == FALSE)
		{
			/* Look through the player's inventory */

			if (removeInventoryItemByObjectiveName(self->requires) == TRUE)
			{
				setInfoBoxMessage(60, 255, 255, 255, _("Used %s"), _(self->requires));

				self->action = &moveToTarget;

				self->active = TRUE;
			}

			else
			{
				setInfoBoxMessage(60, 255, 255, 255, _("%s is needed to open this door"), _(self->requires));
			}
		}
	}

	else if (other->type == PLAYER && self->active == FALSE && self->mental == 0)
	{
		setInfoBoxMessage(60, 255, 255, 255, _("This door is locked"));
	}
}

static void moveToTarget()
{
	if (self->type == AUTO_DOOR)
	{
		self->targetX = self->active == TRUE ? self->endX : self->startX;
		self->targetY = self->active == TRUE ? self->endY : self->startY;
	}

	if (abs(self->x - self->targetX) > self->speed)
	{
		self->dirX = (self->x < self->targetX ? self->speed : -self->speed);
	}

	else
	{
		self->x = self->targetX;
	}

	if (abs(self->y - self->targetY) > self->speed)
	{
		self->dirY = (self->y < self->targetY ? self->speed : -self->speed);
	}

	else
	{
		self->y = self->targetY;
	}

	if (self->x == self->targetX && self->y == self->targetY)
	{
		if (self->type == MANUAL_DOOR)
		{
			self->action = &entityWait;
		}

		if (self->dirX != 0 || self->dirY != 0)
		{
			playSoundToMap("sound/common/door", -1, self->x, self->y, 0);
		}

		self->dirX = 0;
		self->dirY = 0;
	}

	else
	{
		self->x += self->dirX;
		self->y += self->dirY;
	}
}
