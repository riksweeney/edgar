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
#include "../system/random.h"
#include "../world/explosion.h"
#include "key_items.h"

extern Entity *self, player;
extern Game game;

static void dropBomb(int);
static void entityWait(void);
static void explode(void);
static void startFuse(void);
static void touch(Entity *);
static void resumeNormalFunction(void);
static void miniExplode(void);
static void startMiniFuse(void);
static void fallout(void);

Entity *addBomb(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Bomb");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &startFuse;
	e->touch = &keyItemTouch;
	e->activate = &dropBomb;
	e->resumeNormalFunction = &resumeNormalFunction;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

Entity *addMiniBomb(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Bomb");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &startMiniFuse;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void startFuse()
{
	if (self->mental == 1)
	{
		self->endX = playSoundToMap("sound/item/fuse", -1, self->x, self->y, -1);
	}

	self->action = &entityWait;

	checkToMap(self);
}

static void startMiniFuse()
{
	self->thinkTime = 0;

	self->touch = &touch;

	setEntityAnimation(self, "WALK");

	self->animationCallback = &miniExplode;

	self->active = TRUE;

	self->health = 30;

	self->mental = 1;

	self->action = &entityWait;
}

static void entityWait()
{
	long onGround;

	onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (landedOnGround(onGround) == TRUE)
	{
		self->dirX = 0;
	}
}

static void dropBomb(int val)
{
	if (game.status == IN_GAME && player.element != WATER)
	{
		self->thinkTime = 0;

		self->touch = &touch;

		setEntityAnimation(self, "WALK");

		self->animationCallback = &explode;

		self->active = TRUE;

		self->health = 30;

		self->mental = 1;

		self->action = &startFuse;

		self->fallout = &fallout;

		self->flags |= DO_NOT_PERSIST;

		addEntity(*self, player.x, player.y);

		self->inUse = FALSE;
	}
}

static void explode()
{
	int x, y;
	Entity *e;

	self->flags |= NO_DRAW|FLY|DO_NOT_PERSIST;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		x = self->x + self->w / 2;
		y = self->y + self->h / 2;

		stopSound(self->endX);

		x += (prand() % 32) * (prand() % 2 == 0 ? 1 : -1);
		y += (prand() % 32) * (prand() % 2 == 0 ? 1 : -1);

		e = addExplosion(x, y);

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		if (self->damage == -1)
		{
			e->damage = 1;
		}

		self->health--;

		self->thinkTime = 5;

		if (self->health == 0)
		{
			self->inUse = FALSE;
		}
	}

	self->action = &explode;
}

static void miniExplode()
{
	int x, y;
	Entity *e;

	self->flags |= NO_DRAW|FLY|DO_NOT_PERSIST;

	self->thinkTime--;

	x = self->x + self->w / 2;
	y = self->y + self->h / 2;

	stopSound(self->endX);

	e = addExplosion(x, y);

	e->x -= e->w / 2;
	e->y -= e->h / 2;

	e->damage = 1;

	self->inUse = FALSE;
}

static void touch(Entity *other)
{

}

static void resumeNormalFunction()
{
	self->thinkTime = 0;

	self->touch = &touch;

	setEntityAnimation(self, "WALK");

	self->animationCallback = &explode;

	self->active = TRUE;

	self->health = 30;

	self->mental = 1;

	self->dirX = 0;

	self->dirY = 0;

	self->action = &startFuse;
}

static void fallout()
{
	stopSound(self->endX);

	entityDieNoDrop();
}
