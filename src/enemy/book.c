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
#include "../entity.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../collisions.h"
#include "../geometry.h"
#include "../item/key_items.h"
#include "../system/error.h"

extern Entity *self, player;

static void hover(void);
static void lookForPlayer(void);
static void dartDownInit(void);
static void dartDown(void);
static void dartDownFinish(void);
static void dartReactToBlock(void);
static void castFireInit(void);
static void castFire(void);
static void castFireFinish(void);
static void fireDrop(void);
static void fireMove(void);
static void fireBlock(void);

Entity *addBook(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Book");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void lookForPlayer()
{
	float dirX;

	if (self->dirX == 0)
	{
		self->dirX = self->face == LEFT ? self->speed : -self->speed;
	}

	self->face = self->dirX > 0 ? RIGHT : LEFT;

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->dirX = self->face == LEFT ? self->speed : -self->speed;

		self->face = self->face == LEFT ? RIGHT : LEFT;
	}

	if (player.health > 0 && prand() % 60 == 0)
	{
		self->thinkTime = 0;

		if (collision(self->x + (self->face == RIGHT ? self->w : -160), self->y, 160, 200, player.x, player.y, player.w, player.h) == 1)
		{
			switch (self->mental)
			{
				case 0: /* Ram player */
					self->action = &dartDownInit;

					self->thinkTime = 60;
				break;

				default: /* Cast fire */
					self->action = &castFireInit;

					self->thinkTime = 60;
				break;
			}

			self->dirX = 0;
		}
	}

	hover();
}

static void dartDownInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->targetX = player.x;
		self->targetY = player.y + player.h / 2;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed * 6;
		self->dirY *= self->speed * 6;

		self->action = &dartDown;

		self->reactToBlock = &dartReactToBlock;
	}

	hover();
}

static void dartDown()
{
	if (self->dirY == 0 || self->dirX == 0)
	{
		self->thinkTime = 1;

		self->dirX = self->dirY = 0;

		self->action = &dartDownFinish;
	}

	checkToMap(self);
}

static void dartDownFinish()
{
	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		self->dirY = -self->speed;

		self->flags |= FLY;
	}

	else if (self->thinkTime < 0)
	{
		if (self->dirY == 0 || self->y <= self->startY)
		{
			self->action = &lookForPlayer;

			self->dirX = self->dirY = 0;

			self->reactToBlock = &changeDirection;
		}
	}

	else
	{
		if ((self->flags & ON_GROUND) && self->standingOn == NULL)
		{
			self->dirX = 0;
		}
	}

	checkToMap(self);
}

static void dartReactToBlock()
{
	self->flags &= ~FLY;

	self->dirX = self->x < player.x ? -3 : 3;

	self->dirY = -5;

	self->thinkTime = 60;

	self->action = &dartDownFinish;
}

static void hover()
{
	self->startX++;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 8;
}

static void castFireInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental = 5;

		self->action = &castFire;
	}

	hover();
}

static void castFire()
{
	Entity *e = getFreeEntity();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Fire");
		}

		loadProperties("enemy/fire", e);

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->w -= e->w / 2;
		e->h -= e->h / 2;

		e->action = &fireDrop;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;
		e->reactToBlock = &fireBlock;

		e->face = self->face;

		e->type = ENEMY;

		setEntityAnimation(e, STAND);

		self->mental--;

		if (self->mental <= 0)
		{
			self->thinkTime = 60;

			self->action = &castFireFinish;
		}

		else
		{
			self->thinkTime = 10;
		}
	}
}

static void castFireFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &lookForPlayer;
	}
}

static void fireDrop()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = (self->face == LEFT ? -self->speed : self->speed);

		self->action = &fireMove;
	}
}

static void fireMove()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}

static void fireBlock()
{
	self->inUse = FALSE;
}
