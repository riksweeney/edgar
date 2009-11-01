/*
Copyright (C) 2009 Parallel Realities

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "../headers.h"

#include "../graphics/animation.h"
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "key_items.h"
#include "item.h"
#include "../hud.h"
#include "../inventory.h"
#include "../custom_actions.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../collisions.h"
#include "../system/error.h"

extern Entity *self, entity[MAX_ENTITIES];

static void touch(Entity *);
static void spring(void);
static void wait(void);

Entity *addSpring(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Spring");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &wait;
	e->touch = &touch;
	e->fallout = &itemFallout;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->thinkTime = 0;

	return e;
}

static void wait()
{
	if (!(self->flags & GRABBED))
	{
		self->dirX = self->standingOn != NULL ? self->standingOn->dirX : 0;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	pushEntity(other);

	if (other->standingOn == self)
	{
		self->frameSpeed = 1;

		setEntityAnimation(self, WALK);

		self->thinkTime = 30;

		other->y = self->y - other->h;

		other->dirY = -22;

		self->thinkTime = 30;

		playSoundToMap("sound/item/spring.ogg", -1, self->x, self->y, 0);

		self->action = &spring;
	}
}

static void spring()
{
	if (!(self->flags & GRABBED))
	{
		self->dirX = self->standingOn != NULL ? self->standingOn->dirX : 0;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, STAND);

		self->action = &wait;
	}

	checkToMap(self);
}
