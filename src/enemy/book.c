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
#include "../audio/audio.h"
#include "../entity.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../collisions.h"
#include "../geometry.h"
#include "../item/key_items.h"
#include "../system/error.h"
#include "../player.h"
#include "../projectile.h"

extern Entity *self, player;

static void hover(void);
static void lookForPlayer(void);
static void dartDownInit(void);
static void dartDown(void);
static void dartDownFinish(void);
static void dartReactToBlock(void);
static void castFireInit(void);
static void castFire(void);
static void castFinish(void);
static void fireDrop(void);
static void fireMove(void);
static void fireBlock(void);
static void castIceInit(void);
static void iceTouch(Entity *);

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

	self->thinkTime--;

	if (self->thinkTime < 0)
	{
		self->thinkTime = 0;
	}

	if (self->mental == 3 && self->thinkTime <= 0)
	{
		self->action = &castIceInit;

		self->thinkTime = 60;

		self->dirX = 0;
	}

	else if (player.health > 0 && self->thinkTime == 0 && prand() % 20 == 0)
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

		self->dirX *= self->speed * 10;
		self->dirY *= self->speed * 10;

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
			self->thinkTime = 60;

			self->startY = self->y;

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
	self->startX += 5;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 4;
}

static void castFireInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->endX = 5;

		self->action = &castFire;

		playSoundToMap("sound/enemy/fireball/fireball.ogg", -1, self->x, self->y, 0);
	}

	hover();
}

static void castFire()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Fire");
		}

		loadProperties("enemy/fire", e);

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		e->action = &fireDrop;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;
		e->reactToBlock = &fireBlock;

		e->face = self->face;

		e->type = ENEMY;

		e->flags |= DO_NOT_PERSIST;

		setEntityAnimation(e, STAND);

		self->endX--;

		if (self->endX <= 0)
		{
			self->thinkTime = 60;

			self->action = &castFinish;
		}

		else
		{
			self->thinkTime = 3;
		}
	}
}

static void castFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
        self->dirX = self->face == LEFT ? -self->speed : self->speed;

		if (self->mental == 3)
		{
			self->thinkTime = 120 + prand() % 120;
		}

		else
		{
			self->thinkTime = 60;
		}

		self->action = &lookForPlayer;
	}
}

static void fireDrop()
{
	if (self->flags & ON_GROUND)
	{
		self->dirX = (self->face == LEFT ? -self->speed : self->speed);

		self->action = &fireMove;
	}

	checkToMap(self);
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

static void castIceInit()
{
	float i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		for (i=-3;i<=3;i+=1.5f)
		{
			if (i == 0)
			{
				continue;
			}

			e = addProjectile("enemy/ice", self, 0, 0, i, ITEM_JUMP_HEIGHT);

			e->x = self->x + self->w / 2;
			e->y = self->y + self->h / 2;

			e->x -= e->w / 2;
			e->y -= e->h / 2;

			e->draw = &drawLoopingAnimationToMap;
			e->touch = &iceTouch;

			e->face = self->face;

			setEntityAnimation(e, STAND);
		}

		self->thinkTime = 60;

		self->action = &castFinish;
	}

	hover();
}

static void iceTouch(Entity *other)
{
	if (other->type == PLAYER && other->element != ICE && !(other->flags & INVULNERABLE) && other->health > 0)
	{
		setPlayerFrozen(120);

		self->inUse = FALSE;
	}
}
