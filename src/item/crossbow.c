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
#include "../event/script.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../world/weak_wall.h"
#include "key_items.h"

extern Entity *self;

static void entityWait(void);
static void touch(Entity *);
static void activate(int);
static void addBolt(void);
static void addRope(void);

Entity *addCrossbow(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Crossbow");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &addBolt;

	e->activate = &activate;

	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	e->thinkTime = 0;

	return e;
}

static void addBolt()
{
	Entity *e;

	if (self->mental <= 1)
	{
		e = getEntityByObjectiveName("CROSSBOW_BOLT");

		if (e == NULL)
		{
			e = addKeyItem("item/crossbow_bolt", 0, 0);
		}

		e->x = self->x + self->offsetX;

		e->y = self->y + self->offsetY;

		e->startX = e->x;

		e->startY = e->y;

		self->target = e;
	}

	self->action = &entityWait;
}

static void entityWait()
{
	checkToMap(self);

	if (self->mental == 1)
	{
		playSoundToMap("sound/item/crossbow", -1, self->x, self->y, 0);

		self->target->dirX = 12;

		addRope();

		self->mental = 2;
	}
}

static void addRope()
{
	int i, x;
	float dirX;
	Entity *e, *prev;

	dirX = self->target->dirX;

	x = self->target->x;

	prev = self->target;

	for (i=0;i<30;i++)
	{
		e = addWeakWall("item/rope_part", 0, 0);

		e->x = x;

		e->dirX = dirX;

		e->y = self->target->y;

		x -= e->w;

		prev->target = e;

		prev = e;

		e->touch = NULL;
	}
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && self->mental != 2)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
	}
}

static void activate(int val)
{
	if (self->mental != 2)
	{
		runScript("crossbow");
	}
}
