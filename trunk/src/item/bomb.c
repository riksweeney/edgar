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
#include "../custom_actions.h"
#include "../collisions.h"
#include "key_items.h"
#include "../system/random.h"
#include "../world/explosion.h"

extern Entity *self, player;

static void dropBomb(int);
static void wait(void);
static void explode(void);
static void startFuse(void);
static void touch(Entity *);

Entity *addBomb(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Bomb\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &startFuse;
	e->touch = &keyItemTouch;
	e->activate = &dropBomb;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void startFuse()
{
	self->targetX = playSoundToMap("sound/item/fuse.ogg", -1, self->x, self->y, -1);

	self->action = &wait;
}

static void wait()
{
	checkToMap(self);
}

static void dropBomb(int val)
{
	self->thinkTime = 0;

	self->touch = &touch;

	setEntityAnimation(self, WALK);

	self->animationCallback = &explode;

	self->active = TRUE;

	self->health = 10;

	addEntity(*self, player.x, player.y);

	self->inUse = FALSE;
}

static void explode()
{
	int x, y;

	self->flags |= NO_DRAW|FLY;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		x = self->x;
		y = self->y;

		stopSound(self->targetX);

		x += (prand() % 20) * (prand() % 2 == 0 ? 1 : -1);
		y += (prand() % 20) * (prand() % 2 == 0 ? 1 : -1);

		addExplosion(x, y);

		self->health--;

		self->thinkTime = 15;

		if (self->health == 0)
		{
			self->inUse = FALSE;
		}
	}

	self->action = &explode;
}

static void touch(Entity *other)
{

}
