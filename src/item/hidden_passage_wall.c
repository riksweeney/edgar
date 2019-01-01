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
static void sink(void);
static void activate(int);

Entity *addHiddenPassageWall(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Hidden Passage Wall");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &entityWait;
	e->touch = &touch;
	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && self->active == TRUE)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
	}
}

static void activate(int val)
{
	if (self->active == TRUE)
	{
		runScript("hidden_passage");
	}
}

static void entityWait()
{
	if (self->mental == 1)
	{
		self->action = &sink;
	}

	checkToMap(self);
}

static void sink()
{
	self->y += 0.25;

	if (self->y >= self->endY)
	{
		self->inUse = FALSE;
	}

	if (prand() % 2 == 0)
	{
		addSmoke(self->x + prand() % self->w, self->endY - prand() % 10, "decoration/dust");
	}
}
