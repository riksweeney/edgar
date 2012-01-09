/*
Copyright (C) 2009-2012 Parallel Realities

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
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void entityWait(void);
static void activate(int);
static void touch(Entity *);

Entity *addGrapplingHook(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Light Target");
	}

	loadProperties(name, e);

	e->touch = &touch;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;

	e->activate = &activate;

	e->x = x;
	e->y = y;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	checkToMap(self);
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
	if (self->mental == 0)
	{
		runScript("grappling_hook");
	}

	else
	{
		self->dirX = self->speed;

		self->action = &draw;

		self->action = &fireHook;
	}
}

static void fireHook()
{
	checkToMap(self);

	if (self->dirX == 0)
	{

	}

	self->action = &entityWait;
}

static int draw()
{
	return TRUE;
}
