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
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self, player;

static void init(void);
static void entityWait(void);
static void fadeOut(void);
static void fadeIn(void);

Entity *addForceField(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Force Field");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &pushEntity;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	if (self->active != self->health)
	{
		if (self->active == TRUE)
		{
			self->action = &fadeIn;
		}

		else
		{
			self->flags |= DO_NOT_PERSIST;

			if (self->mental == -1)
			{
				playSoundToMap("sound/item/force_field_shutdown", -1, self->x, self->y, 0);
			}

			self->action = &fadeOut;
		}

		self->active = self->health;
	}

	checkToMap(self);
}

static void fadeOut()
{
	self->alpha -= 3;

	if (self->alpha <= 0)
	{
		self->alpha = 0;

		self->touch = NULL;

		self->action = &entityWait;

		self->inUse = FALSE;
	}
}

static void fadeIn()
{
	self->alpha += 3;

	if (self->alpha >= 128)
	{
		self->alpha = 128;

		self->touch = &pushEntity;

		self->action = &entityWait;

		self->active = TRUE;
	}
}

static void init()
{
	self->health = self->active;

	if (self->active == TRUE)
	{
		self->alpha = 128;
	}

	else
	{
		self->flags |= NO_DRAW;

		self->touch = NULL;
	}

	self->action = &entityWait;
}
