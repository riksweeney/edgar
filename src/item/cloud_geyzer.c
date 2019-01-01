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

#include "../collisions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void init(void);
static void entityWait(void);
static void touch(Entity *);
static void cloudTouch(Entity *);
static void cloudWait(void);

Entity *addCloudGeyzer(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Cloud Geyzer");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

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
	char name[MAX_VALUE_LENGTH];
	Entity *e;

	snprintf(name, MAX_VALUE_LENGTH, "%s_CLOUD", self->objectiveName);

	e = getEntityByObjectiveName(name);

	if (e == NULL)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Cloud Geyzer Cloud");
		}

		loadProperties("item/cloud_geyzer_cloud", e);

		e->x = self->x + self->w / 2 - e->w / 2;

		e->y = self->y;
		e->y -= 128;

		STRNCPY(e->objectiveName, name, sizeof(e->objectiveName));
	}

	e->face = RIGHT;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &cloudTouch;

	e->action = &cloudWait;

	setEntityAnimation(e, "STAND");

	self->target = e;

	self->action = &entityWait;
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = self->maxThinkTime;

		self->target->dirY = -self->mental;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	char name[MAX_VALUE_LENGTH];

	if (other->dirY > 0)
	{
		snprintf(name, MAX_VALUE_LENGTH, "%s_CLOUD", self->name);

		if (other->type == ITEM && strcmpignorecase(other->name, name) == 0)
		{
			other->y = self->y;
			other->y -= other->h;

			other->standingOn = self;
			other->dirY = 0;
			other->flags |= ON_GROUND;
		}
	}
}

static void cloudTouch(Entity *other)
{
	int bottomBefore;

	if (other->dirY > 0)
	{
		/* Trying to move down */

		if (collision(other->x, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			bottomBefore = other->y + other->h - other->dirY - 1;

			if (abs(bottomBefore - self->y) < self->h - 1)
			{
				/* Place the player as close to the solid tile as possible */

				other->y = self->y;
				other->y -= other->h;

				other->standingOn = self;
				other->dirY = 0;
				other->flags |= ON_GROUND;
			}
		}
	}
}

static void cloudWait()
{
	checkToMap(self);
}
