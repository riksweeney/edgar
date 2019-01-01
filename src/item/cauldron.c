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
#include "../event/script.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void entityWait(void);
static void touch(Entity *);
static void activate(int);
static void shudder(void);

Entity *addCauldron(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Cauldron");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &entityWait;

	e->activate = &activate;

	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	e->thinkTime = 0;

	return e;
}

static void entityWait()
{
	self->x = self->startX;

	if (self->mental == 1)
	{
		self->action = &shudder;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && other->action == NULL)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
	}
}

static void activate(int val)
{
	runScript("cauldron");
}

static void shudder()
{
	Entity *smoke;

	self->endX += 90;

	if (self->endX >= 360)
	{
		self->endX = 0;
	}

	self->x = self->startX + sin(DEG_TO_RAD(self->endX)) * 4;

	checkToMap(self);

	if (self->mental == 2)
	{
		smoke = addSmoke(0, 0, "decoration/dust");

		if (smoke != NULL)
		{
			smoke->x = self->x + prand() % self->w;
			smoke->y = self->y + prand() % self->h;

			smoke->dirY = 0;
		}
	}

	else if (self->mental == 0)
	{
		self->x = self->startX;

		self->action = &entityWait;
	}
}
