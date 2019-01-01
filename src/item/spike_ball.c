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
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "key_items.h"

extern Entity *self, player;
extern Game game;

static void entityWait(void);
static void explode(void);
static void throwBall(int);
static void touch(Entity *);

Entity *addSpikeBall(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Spike Ball");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->activate = &throwBall;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void throwBall(int val)
{
	Entity *e;

	if (self->thinkTime <= 0 && game.status == IN_GAME && player.element != WATER)
	{
		setEntityAnimation(self, "WALK");

		self->active = TRUE;

		e = addEntity(*self, player.x + (player.face == RIGHT ? player.w : 0), player.y);

		e->thinkTime = 120;

		e->flags |= DO_NOT_PERSIST;

		e->touch = &touch;

		e->action = &entityWait;

		e->dirX = player.face == LEFT ? -8 : 8;

		e->dirY = ITEM_JUMP_HEIGHT;

		e->fallout = &entityDieNoDrop;

		playSoundToMap("sound/common/throw", -1, player.x, player.y, 0);

		self->health--;

		if (self->health <= 0)
		{
			self->inUse = FALSE;
		}

		else
		{
			self->thinkTime = 120;
		}
	}
}

static void entityWait()
{
	long onGround;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &explode;

		self->flags |= FLY;

		self->dirY = -4;

		self->thinkTime = 15;
	}

	onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (landedOnGround(onGround) == TRUE)
	{
		self->dirX = 0;
	}
}

static void explode()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		for (i=0;i<360;i+=12)
		{
			e = addProjectile("weapon/spike", &player, 0, 0, 0, 0);

			e->x = self->x + self->w / 2 - e->w / 2;
			e->y = self->y + self->h / 2 - e->h / 2;

			e->dirX = (0 * cos(DEG_TO_RAD(i)) - 12 * sin(DEG_TO_RAD(i)));
			e->dirY = (0 * sin(DEG_TO_RAD(i)) + 12 * cos(DEG_TO_RAD(i)));

			e->face = e->dirX < 0 ? LEFT : RIGHT;

			e->flags |= FLY|ATTACKING;

			e->reactToBlock = &bounceOffShield;
		}

		self->inUse = FALSE;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{

}
