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
#include "key_items.h"

extern Entity *self, player;
extern Game game;

static void sprayRepellent(int);
static void sprayMove(void);
static void sprayTouch(Entity *);

Entity *addRepellent(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Repellent");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->activate = &sprayRepellent;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void sprayRepellent(int val)
{
	Entity *e;

	if (self->thinkTime <= 0 && game.status == IN_GAME && player.element != WATER)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Repellent Spray");
		}

		loadProperties("item/repellent_spray", e);

		e->x = player.x + (player.face == RIGHT ? player.w : 0);
		e->y = player.y + player.h / 2;

		setEntityAnimation(e, "STAND");

		e->x -= player.face == RIGHT ? e->box.x : e->box.x + e->box.w;

		e->y -= e->h / 2;

		e->type = ITEM;

		e->face = RIGHT;

		e->action = &sprayMove;
		e->touch = &sprayTouch;

		e->draw = &drawLoopingAnimationToMap;

		e->active = FALSE;

		self->thinkTime = self->maxThinkTime;

		e->dirX = player.face == RIGHT ? 2 + player.speed : -2 - player.speed;

		e->thinkTime = 30;

		e->flags |= DO_NOT_PERSIST;

		playSoundToMap("sound/item/spray", -1, player.x, player.y, 0);
	}
}

static void sprayTouch(Entity *other)
{
	Entity *temp;

	if (other->type == ENEMY && strcmpignorecase(other->name, "huge_spider") == 0)
	{
		temp = self;

		self = other;

		self->takeDamage(temp, temp->damage);

		self = temp;
	}
}

static void sprayMove()
{
	self->dirX *= 0.95;

	if (self->mental < 2)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->mental++;

			setEntityAnimation(self, self->mental == 1 ? "WALK" : "JUMP");

			self->thinkTime = self->mental == 2 ? 360 : 30;
		}
	}

	if (fabs(self->dirX) <= 0.05)
	{
		self->dirX = 0;

		self->dirY = -self->speed;
	}

	checkToMap(self);

	if (self->mental == 2)
	{
		self->thinkTime--;

		if (self->thinkTime < 90)
		{
			if (self->thinkTime % 3 == 0)
			{
				self->flags ^= NO_DRAW;
			}
		}

		if (self->thinkTime <= 0)
		{
			self->inUse = FALSE;
		}
	}
}
