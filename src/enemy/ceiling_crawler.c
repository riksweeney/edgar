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
#include "../map.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void moveOnFloor(void);
static void moveOnCeiling(void);
static void changeFloor(void);
static int safeToDrop(void);
static int safeToRise(void);
static void attackInit(void);
static void attackWait(void);
static void fireShot(void);
static void attackFinished(void);
static void riseUp(void);
static void changeWait(void);
static void creditsMove(void);
static void die(void);

Entity *addCeilingCrawler(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Ceiling Crawler");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &changeFloor;
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

static void moveOnFloor()
{
	moveLeftToRight();

	if (prand() % 600 == 0)
	{
		self->action = &attackInit;
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (safeToRise() == TRUE)
			{
				self->dirX = 0;

				self->thinkTime = 60;

				self->action = &changeWait;
			}

			else
			{
				self->thinkTime = self->maxThinkTime;
			}
		}
	}
}

static void die()
{
	playSoundToMap("sound/enemy/ceiling_crawler/ceiling_crawler_die", -1, self->x, self->y, 0);

	entityDie();
}

static void moveOnCeiling()
{
	if (self->dirX == 0)
	{
		self->face = self->face == RIGHT ? LEFT : RIGHT;
	}

	self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

	if (isAtCeilingEdge(self) == TRUE)
	{
		self->dirX = 0;
	}

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	if (prand() % 300 == 0)
	{
		self->action = &attackInit;
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (safeToDrop() == TRUE)
			{
				self->dirX = 0;

				self->thinkTime = 60;

				self->action = &changeWait;
			}

			else
			{
				self->thinkTime = self->maxThinkTime;
			}
		}
	}
}

static void changeWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags ^= FLY;

		if (self->flags & FLY)
		{
			self->dirY = -GRAVITY_SPEED * self->weight;
		}

		self->action = &changeFloor;

		self->creditsAction = &changeFloor;
	}

	checkToMap(self);
}

static void changeFloor()
{
	checkToMap(self);

	if (self->dirY == 0 || self->standingOn != NULL)
	{
		self->thinkTime = self->maxThinkTime;

		self->action = !(self->flags & FLY) ? &moveOnFloor : &moveOnCeiling;

		self->creditsAction = &creditsMove;

		setEntityAnimation(self, !(self->flags & FLY) ? "STAND" : "WALK");

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}

	else if (self->flags & FLY)
	{
		self->dirY -= GRAVITY_SPEED * self->weight;
	}
}

static int safeToDrop()
{
	int x, y, i, tile;

	x = self->x + self->w / 2;

	y = self->y + self->h - 1;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	y++;

	for (i=0;i<15;i++)
	{
		tile = mapTileAt(x, y);

		if (tile != BLANK_TILE && (tile < BACKGROUND_TILE_START || tile > FOREGROUND_TILE_START))
		{
			return tile < BACKGROUND_TILE_START ? TRUE : FALSE;
		}

		y++;
	}

	return FALSE;
}

static int safeToRise()
{
	int x, y, i, tile;

	x = self->x + self->w / 2;

	y = self->y;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	y--;

	for (i=0;i<15;i++)
	{
		tile = mapTileAt(x, y);

		if (tile != BLANK_TILE && (tile < BACKGROUND_TILE_START || tile > FOREGROUND_TILE_START))
		{
			return tile < BACKGROUND_TILE_START ? TRUE : FALSE;
		}

		y--;
	}

	return FALSE;
}

static void attackInit()
{
	self->dirX = 0;

	setEntityAnimation(self, !(self->flags & FLY) ? "ATTACK_1" : "ATTACK_3");

	self->animationCallback = &fireShot;
}

static void fireShot()
{
	Entity *e;

	self->action = &attackWait;

	self->frameSpeed = 0;

	self->thinkTime = 60;

	e = addProjectile("common/green_blob", self, self->x + self->w / 2, self->y, 0, 3 * (self->flags & FLY) ? 1 : -1);

	e->x -= e->w / 2;

	e->flags |= FLY;

	e->y += !(self->flags & FLY) ? 5 : (self->h - 5);

	e->type = ENEMY;

	e->parent = NULL;

	e->action = &riseUp;

	e->flags |= DO_NOT_PERSIST;

	e->head = self;

	e->thinkTime = 5;
}

static void attackWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->frameSpeed = 1;

		setEntityAnimation(self, !(self->flags & FLY) ? "ATTACK_2" : "ATTACK_4");

		self->animationCallback = &attackFinished;
	}

	checkToMap(self);
}

static void attackFinished()
{
	setEntityAnimation(self, !(self->flags & FLY) ? "STAND" : "WALK");

	self->thinkTime = self->maxThinkTime;

	self->action = !(self->flags & FLY) ? &moveOnFloor : &moveOnCeiling;

	self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
}

static void riseUp()
{
	Entity *e;
	int x, y, dirY;

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		x = self->x;
		y = self->y;

		dirY = self->dirY > 0 ? 1 : -1;

		e = addProjectile("common/green_blob", self->head, x, y, -6, 0);

		e->flags |= FLY;

		e->reactToBlock = &bounceOffShield;

		e = addProjectile("common/green_blob", self->head, x, y, -6, 6 * dirY);

		e->flags |= FLY;

		e->reactToBlock = &bounceOffShield;

		e = addProjectile("common/green_blob", self->head, x, y, 0, 6 * dirY);

		e->flags |= FLY;

		e->reactToBlock = &bounceOffShield;

		e = addProjectile("common/green_blob", self->head, x, y, 6, 6 * dirY);

		e->flags |= FLY;

		e->reactToBlock = &bounceOffShield;

		e = addProjectile("common/green_blob", self->head, x, y, 6, 0);

		e->flags |= FLY;

		e->reactToBlock = &bounceOffShield;

		playSoundToMap("sound/common/pop", -1, self->x, self->y, 0);

		self->inUse = FALSE;
	}
}

static void creditsMove()
{
	self->thinkTime++;

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}

	if (self->thinkTime != 0 && (self->thinkTime % 300) == 0)
	{
		if ((!(self->flags & FLY) && safeToRise() == TRUE) || ((self->flags & FLY) && safeToDrop() == TRUE))
		{
			self->dirX = 0;

			self->thinkTime = 60;

			self->creditsAction = &changeWait;
		}
	}
}
