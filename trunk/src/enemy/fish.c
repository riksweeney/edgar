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
#include "../system/properties.h"
#include "../entity.h"
#include "../map.h"
#include "../geometry.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../player.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../system/error.h"

extern Entity *self, player, entity[MAX_ENTITIES];

static void swim(void);
static void fallout(void);
static void init(void);
static void attackPlayer(void);
static void lookForPlayer(void);
static void returnToStart(void);
static void die(void);
static void moveToFood(void);
static void touch(Entity *);
static void bobOnSurface(void);

Entity *addFish(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Fish");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->fallout = &fallout;

	e->action = &init;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	self->dirY = 0;

	self->endY = getWaterTop(self->startX, self->startY);

	if (self->mental == 1)
	{
		self->endY -= TILE_SIZE;

		self->touch = NULL;

		setEntityAnimation(self, "DIE");

		self->action = &die;
	}

	else
	{
		self->action = &swim;
	}
}

static void swim()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	if (prand() % 5 == 0)
	{
		lookForPlayer();
	}

	if (self->mental == 1)
	{
		self->endY = self->endY - TILE_SIZE;

		self->damage = 0;

		setEntityAnimation(self, "DIE");

		self->touch = NULL;

		self->action = &die;

		fireTrigger(self->objectiveName);

		fireGlobalTrigger(self->objectiveName);
	}

	if (self->y < self->endY)
	{
		self->dirY = 0;

		self->y = self->endY;
	}
}

static void die()
{
	self->dirX = 0;

	self->dirY = -0.5;

	checkToMap(self);

	if (self->y < self->endY)
	{
		self->y = self->endY;

		self->action = &bobOnSurface;
	}
}

static void bobOnSurface()
{
	self->endX++;

	if (self->endX >= 360)
	{
		self->endX = 0;
	}

	self->y = self->endY + sin(DEG_TO_RAD(self->endX)) * 4;
}

static void fallout()
{
	if (self->environment == WATER)
	{
		self->flags |= FLY;

		self->action = self->mental == 1 ? &die : &swim;
	}

	else
	{
		entityDie();
	}
}

static void lookForPlayer()
{
	int i;

	if (player.health > 0 && player.environment == WATER && getDistanceFromPlayer(self) < SCREEN_WIDTH)
	{
		self->thinkTime = 60;

		setEntityAnimation(self, "ATTACK_1");

		self->action = &attackPlayer;
	}

	else
	{
		for (i=0;i<MAX_ENTITIES;i++)
		{
			if (entity[i].inUse == FALSE || strcmpignorecase(entity[i].name, "item/poison_meat") != 0)
			{
				continue;
			}

			if (entity[i].environment == WATER)
			{
				if (getDistance(self->x, self->y, entity[i].x, entity[i].y) < SCREEN_WIDTH)
				{
					self->target = &entity[i];

					setEntityAnimation(self, "ATTACK_1");

					self->action = &moveToFood;

					self->thinkTime = 0;

					break;
				}
			}
		}
	}
}

static void moveToFood()
{
	if (self->target->health <= 0 || self->target->inUse == FALSE)
	{
		self->targetX = self->startX;
		self->targetY = self->startY;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed;
		self->dirY *= self->speed;

		self->face = self->dirX < 0 ? LEFT : RIGHT;

		setEntityAnimation(self, "STAND");

		self->action = &returnToStart;

		self->thinkTime = self->mental == 1 ? 60 + prand() % 180 : 600;

		self->target = NULL;
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->thinkTime = 0;
		}

		calculatePath(self->x, self->y, self->target->x, self->target->y, &self->dirX, &self->dirY);

		self->dirX *= 3;
		self->dirY *= 3;

		self->face = self->dirX < 0 ? LEFT : RIGHT;

		checkToMap(self);

		if (self->y < self->endY)
		{
			self->y = self->endY;
		}
	}
}

static void touch(Entity *other)
{
	if (self->target != NULL && self->target == other)
	{
		if (self->thinkTime <= 0)
		{
			self->target->health--;

			self->thinkTime = 30;

			self->mental = 1;

			if (self->target->health <= 0)
			{
				self->target->inUse = FALSE;
			}
		}
	}

	else
	{
		entityTouch(other);
	}
}

static void attackPlayer()
{
	if (getDistanceFromPlayer(self) > SCREEN_WIDTH || player.health <= 0 || player.environment == AIR)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0 || player.health <= 0 || player.environment == AIR)
		{
			self->targetX = self->startX;
			self->targetY = self->startY;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= self->speed;
			self->dirY *= self->speed;

			self->face = self->dirX < 0 ? LEFT : RIGHT;

			setEntityAnimation(self, "STAND");

			self->action = &returnToStart;

			self->thinkTime = 600;
		}
	}

	else
	{
		self->thinkTime = 60;

		calculatePath(self->x, self->y, player.x, player.y, &self->dirX, &self->dirY);

		self->dirX *= 3;
		self->dirY *= 3;

		self->face = self->dirX < 0 ? LEFT : RIGHT;

		checkToMap(self);

		if (self->y < self->endY)
		{
			self->y = self->endY;
		}
	}
}

static void returnToStart()
{
	checkToMap(self);

	self->thinkTime--;

	if (atTarget() || self->thinkTime <= 0)
	{
		self->dirY = 0;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->action = &swim;
	}
}
