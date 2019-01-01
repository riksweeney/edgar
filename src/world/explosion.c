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
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void finish(void);
static void explode(void);
static void die(void);

Entity *addExplosion(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Explosion");
	}

	loadProperties("common/explosion", e);

	e->x = x;
	e->y = y;

	e->flags |= FLY|ATTACKING;

	e->face = RIGHT;

	e->action = &explode;
	e->touch = &entityTouch;

	e->die = &die;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	e->animationCallback = &finish;

	return e;
}

static void explode()
{
	if (self->mental == 0)
	{
		playSoundToMap("sound/common/explosion", -1, self->x, self->y, 0);
	}

	self->action = &doNothing;
}

static void finish()
{
	self->inUse = FALSE;
}

static void die()
{
	/* Do nothing */
}
