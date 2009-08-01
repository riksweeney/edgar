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
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../geometry.h"

extern Entity *self, player;

static void fly(void);
static void die(void);
static void pain(void);
static void dartDownInit(void);
static void dartDown(void);
static void dartDownFinish(void);
static void dartReactToBlock(void);

Entity *addBat(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Bat\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->pain = &pain;

	if (strcmpignorecase(name, "enemy/red_bat") == 0)
	{
		e->takeDamage = &entityTakeDamageNoFlinch;
	}

	else
	{
		e->takeDamage = &entityTakeDamageFlinch;
	}

	e->reactToBlock = &changeDirection;

	e->action = &fly;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void die()
{
	entityDie();
}

static void pain()
{
	playSoundToMap("sound/enemy/bat/squeak.ogg", -1, self->x, self->y, 0);
}

static void fly()
{
	int speed;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (strcmpignorecase(self->name, "enemy/red_bat") == 0)
		{
			switch (prand() % 2)
			{
				case 0:
					self->dirX = self->speed;
				break;

				default:
					self->dirX = -self->speed;
				break;
			}
		}

		else
		{
			switch (prand() % 5)
			{
				case 0:
				case 1:
					self->dirX = self->speed;
				break;

				case 2:
				case 3:
					self->dirX = -self->speed;
				break;

				default:
					self->dirX = 0;
				break;
			}
		}

		self->thinkTime = 180 + prand() % 120;
	}

	if (self->dirX < 0)
	{
		self->face = LEFT;
	}

	else if (self->dirX > 0)
	{
		self->face = RIGHT;
	}
	
	/* Red bats always try and stay at their minimum height */
	
	if (self->y > self->startY && strcmpignorecase(self->name, "enemy/red_bat") == 0)
	{
		self->dirY = -self->speed;
	}
	
	else
	{
		self->dirY = 0;
	}

	speed = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && speed != 0)
	{
		self->dirX = speed * -1;
	}

	if (player.health > 0 && prand() % 120 == 0 && (strcmpignorecase(self->name, "enemy/red_bat") == 0))
	{
		if (collision(self->x + (self->face == RIGHT ? self->w : -160), self->y, 160, 200, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &dartDownInit;

			self->thinkTime = 60;

			self->dirX = 0;
		}
	}
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
			self->action = &fly;

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
