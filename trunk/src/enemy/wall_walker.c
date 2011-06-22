/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "../entity.h"
#include "../collisions.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../system/error.h"

extern Entity *self, player;

static void walkOnGround(void);
static void walkUpWall(void);
static void walkOnCeiling(void);
static void walkDownWall(void);
static void die(void);

Entity *addWallWalker(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Wall Walker");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &walkOnGround;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void walkOnGround()
{
	checkToMap(self);

	if (isAtEdge(self) == TRUE)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	if (self->dirX == 0)
	{
		self->flags |= FLY;

		self->action = &walkUpWall;

		self->dirY = -self->speed;
	}
}

static void walkUpWall()
{
	checkToMap(self);

	if (self->dirY == 0)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->action = &walkOnCeiling;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}
}

static void walkOnCeiling()
{
	float dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0)
	{
		if (dirX == 0)
		{
			self->face = self->face == LEFT ? RIGHT : LEFT;

			self->dirX = self->face == LEFT ? -self->speed : self->speed;
		}

		else
		{
			self->action = &walkDownWall;

			self->dirY = self->speed;
		}
	}
}

static void walkDownWall()
{
	checkToMap(self);

	if (self->dirY == 0)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->flags &= ~FLY;

		self->action = &walkOnGround;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}
}

static void die()
{
	Entity *arrow;

	/* Drop between 1 and 3 arrows */

	arrow = addTemporaryItem("weapon/normal_arrow", self->x, self->y, RIGHT, 0, ITEM_JUMP_HEIGHT);

	arrow->health = 1 + (prand() % 3);

	entityDieNoDrop();
}
