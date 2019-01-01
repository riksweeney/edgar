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
#include "../custom_actions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "item.h"
#include "key_items.h"

extern Entity *self, player;
extern Game game;

static void grab(int);
static void extend(void);
static void retract(void);
static void touch(Entity *);
static int draw(void);

Entity *addExtendOGrab(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Extend-O-Grab");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->activate = &grab;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void grab(int val)
{
	Entity *e;

	if (self->thinkTime <= 0 && game.status == IN_GAME && (player.flags & ON_GROUND) && isAttacking() == FALSE && player.element != WATER)
	{
		setCustomAction(&player, &helpless, 2, 0, 0);

		playerStand();

		player.dirX = 0;

		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add an Extend-O-Grab");
		}

		loadProperties(self->name, e);

		setEntityAnimation(e, "WALK");

		e->flags |= FLY;

		e->x = player.x + player.w / 2;
		e->y = player.y + player.h / 2 - e->h / 2;

		e->dirX = player.face == LEFT ? -self->speed : self->speed;

		e->startX = e->x;

		e->face = player.face;

		e->action = &extend;
		e->touch = &touch;
		e->draw = &draw;

		e->health = 1;

		self->thinkTime = self->maxThinkTime;
	}
}

static void extend()
{
	setCustomAction(&player, &helpless, 2, 0, 0);

	if (self->dirX == 0)
	{
		self->dirX = self->face == LEFT ? self->speed : -self->speed;

		self->action = &retract;

		setEntityAnimation(self, "JUMP");

		self->health = 2;
	}

	else
	{
		checkToMap(self);

		self->mental += fabs(self->dirX);

		if (self->mental >= 256)
		{
			self->dirX = 0;
		}
	}

	self->y = player.y + player.h / 2 - self->h / 2;
}

static void retract()
{
	setCustomAction(&player, &helpless, 2, 0, 0);

	self->x += self->face == LEFT ? self->speed : -self->speed;

	if (self->target != NULL)
	{
		self->target->x += self->face == LEFT ? self->speed : -self->speed;

		self->target->y = self->y + self->h / 2 - self->target->h / 2;
	}

	if ((self->face == RIGHT && self->x <= self->startX) || (self->face == LEFT && self->x >= self->startX))
	{
		self->target = NULL;

		self->inUse = FALSE;
	}

	self->y = player.y + player.h / 2 - self->h / 2;
}

static void touch(Entity *other)
{
	if (self->target == NULL && (other->touch == &keyItemTouch || other->touch == &healthTouch))
	{
		self->target = other;

		self->target->x = self->x + self->w / 2 - other->w / 2;

		self->target->y = self->y + self->h / 2 - other->h / 2;

		self->dirX = 0;
	}
}

static int draw()
{
	int startX;

	startX = self->x;

	/* Draw the segments first */

	setEntityAnimation(self, "BLOCK");

	if (self->face == RIGHT)
	{
		while (self->x >= self->startX)
		{
			drawSpriteToMap();

			self->x -= self->w;
		}
	}

	else
	{
		while (self->x <= self->startX)
		{
			drawSpriteToMap();

			self->x += self->w;
		}
	}

	/* Draw the tip */

	setEntityAnimation(self, self->health == 1 ? "WALK" : "JUMP");

	self->x = startX;

	drawLoopingAnimationToMap();

	return TRUE;
}
