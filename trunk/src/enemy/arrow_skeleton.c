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
#include "../projectile.h"
#include "../system/error.h"
#include "../audio/audio.h"

extern Entity *self, player;

static void lookForPlayer(void);
static void readyArrow(void);
static void fireArrow(void);
static void fireArrowWait(void);
static void creditsMove(void);
static void die(void);

Entity *addArrowSkeleton(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Arrow Skeleton");
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

			facePlayer();

			self->thinkTime = 30;

			self->action = &readyArrow;
		}
	}
}

static void readyArrow()
{
	self->dirX = 0;

	facePlayer();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "FIRE_ARROW");

		self->animationCallback = &fireArrow;

		self->thinkTime = 15;

		self->action = &fireArrowWait;
	}

	checkToMap(self);
}

static void fireArrowWait()
{
	checkToMap(self);
}

static void fireArrow()
{
	Entity *e;

	e = addProjectile("weapon/flaming_arrow", self, self->x + (self->face == RIGHT ? 0 : self->w), self->y + 27, self->face == RIGHT ? 12 : -12, 0);

	e->damage = 1;

	if (e->face == LEFT)
	{
		e->x -= e->w;
	}

	playSoundToMap("sound/enemy/fireball/fireball.ogg", -1, self->x, self->y, 0);

	e->reactToBlock = &bounceOffShield;

	e->face = self->face;

	e->flags |= FLY;

	setEntityAnimation(self, "FIRE_ARROW_FINISH");

	self->animationCallback = &fireArrowFinish;

	checkToMap(self);
}

static void fireArrowFinish()
{
	self->action = &fireArrowFinish;

	setEntityAnimation(self, "STAND");

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental--;

		if (self->mental <= 0)
		{
			self->action = &lookForPlayer;

			self->thinkTime = 180;
		}

		else
		{
			self->action = &readyArrow;
		}
	}

	checkToMap(self);
}
