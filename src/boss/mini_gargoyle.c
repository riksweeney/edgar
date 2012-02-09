/*
Copyright (C) 2009-2012 Parallel Realities

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
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void attackPlayer(void);
static void touch(Entity *);
static void takeDamage(Entity *, int);
static void die(void);
static void dieWait(void);
static void moveToGargoyleInit(void);
static void moveToGargoyle(void);
static void stickAndWait(void);
static void raiseOffScreen(void);

Entity *addMiniGargoyle(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Mini Gargoyle");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &attackPlayer;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->takeDamage = &takeDamage;
	e->touch = &touch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void attackPlayer()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		facePlayer();

		self->targetX = player.x + player.w / 2 - self->w / 2;
		self->targetY = player.y + player.h / 2 - self->h / 2;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed;
		self->dirY *= self->speed;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	if (self->health <= 0)
	{
		return;
	}

	if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}
	}

	else if (other->type == PROJECTILE && other->parent != self)
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}

		other->inUse = FALSE;
	}

	else if (other->type == PLAYER && !(self->flags & GRABBING))
	{
		self->startX = prand() % other->w;

		self->startY = prand() % other->h;

		self->head->endX--;

		self->action = &stickAndWait;

		self->thinkTime = 120;

		self->layer = FOREGROUND_LAYER;
	}
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

			if (self->pain != NULL)
			{
				self->pain();
			}
		}

		else
		{
			self->damage = 0;

			self->die();
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}
	}
}

static void die()
{
	self->flags &= ~FLY;

	self->takeDamage = NULL;

	self->damage = 0;

	self->action = &dieWait;

	self->head->mental--;

	self->head->endX = 0;
}

static void dieWait()
{
	if (self->head->mental <= 0)
	{
		self->thinkTime = 60;

		self->action = &moveToGargoyleInit;

		self->flags |= FLY;

		self->mental = 1;
	}

	checkToMap(self);
}

static void moveToGargoyleInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->head->endX++;

		self->targetX = self->head->x;
		self->targetY = self->head->y;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed;
		self->dirY *= self->speed;

		self->action = &moveToGargoyle;
	}

	checkToMap(self);
}

static void moveToGargoyle()
{
	if (atTarget())
	{
		if (self->mental == 1)
		{
			self->mental = 0;

			self->head->endX--;

			self->thinkTime = 60;
		}

		else if (self->head->endX <= 0)
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->head->flags &= ~NO_DRAW;

				self->inUse = FALSE;
			}
		}
	}
}

static void stickAndWait()
{
	self->x = player.x + self->startX;
	self->y = player.x + self->startY;

	if (self->head->endX <= 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->targetX = player.x;
			self->targetY = self->y - SCREEN_HEIGHT;

			self->action = &raiseOffScreen;
		}
	}

	checkToMap(self);
}

static void raiseOffScreen()
{
	self->y -= 3;

	player.x = self->targetX;
	player.y = self->y - self->startY;

	if (self->y < self->targetY)
	{
		freeEntityList(playerGib());

		self->inUse = FALSE;
	}
}
