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
#include "../system/random.h"

extern Entity *self, player;

static void createSnowball(void);
static void lookForPlayer(void);
static void throwSnowball(void);
static void creditsMove(void);
static void die(void);

Entity *addSasquatch(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Sasquatch");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void die()
{
	playSoundToMap("sound/enemy/gazer/gazer_die", -1, self->x, self->y, 0);

	entityDie();
}

static void lookForPlayer()
{
	setEntityAnimation(self, "WALK");

	moveLeftToRight();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}

	if (player.health > 0 && self->thinkTime <= 0)
	{
		/* Must be within a certain range */

		if (collision(self->x + (self->face == LEFT ? -300 : self->w), self->y, 300, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->dirX = 0;

			self->thinkTime = 30;

			self->mental = 1 + prand() % 3;

			setEntityAnimation(self, "ATTACK_1");

			self->action = &createSnowball;
		}
	}
}

static void createSnowball()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "ATTACK_2");

		self->action = &throwSnowball;

		self->creditsAction = &throwSnowball;

		e = addProjectile("enemy/sasquatch_snowball", self, self->x, self->y, (self->face == RIGHT ? 8 : -8), 0);

		playSoundToMap("sound/common/throw", -1, self->x, self->y, 0);

		e->reactToBlock = &bounceOffShield;

		e->flags |= FLY;

		e->x += (self->face == RIGHT ? self->w : e->w);
		e->y += self->offsetY;

		self->thinkTime = 15;
	}

	checkToMap(self);
}

static void throwSnowball()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental--;

		if (self->mental > 0)
		{
			self->thinkTime = 30;

			setEntityAnimation(self, "ATTACK_1");

			self->action = &createSnowball;

			self->creditsAction = &createSnowball;
		}

		else
		{
			self->dirX = self->face == LEFT ? -self->speed : self->speed;

			self->thinkTime = 120;

			self->action = &lookForPlayer;

			self->creditsAction = &creditsMove;
		}
	}
}

static void creditsMove()
{
	self->thinkTime--;

	setEntityAnimation(self, "WALK");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}

	if (self->thinkTime <= 0)
	{
		self->dirX = 0;

		self->thinkTime = 30;

		self->mental = 1 + prand() % 3;

		setEntityAnimation(self, "ATTACK_1");

		self->creditsAction = &createSnowball;
	}
}
