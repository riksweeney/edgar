/*
Copyright (C) 2009-2010 Parallel Realities

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
#include "../system/error.h"

extern Entity *self, player;

static void swim(void);
static void fallout(void);
static void init(void);
static void attackPlayer(void);
static void lookForPlayer(void);
static void returnToStart(void);

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
	e->touch = &entityTouch;
	e->fallout = &fallout;

	e->action = &init;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	self->endY = getWaterTop(self->x, self->y);
	
	printf("Water top is %f\n", self->endY);
	
	self->action = &swim;
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
}

static void fallout()
{
	if (self->environment == WATER)
	{
		self->flags |= FLY;

		self->action = &swim;
	}

	else
	{
		entityDie();
	}
}

static void lookForPlayer()
{
	if (player.health > 0 && player.environment == WATER && getDistanceFromPlayer(self) < SCREEN_WIDTH)
	{
		self->thinkTime = 60;
		
		setEntityAnimation(self, ATTACK_1);
		
		self->action = &attackPlayer;
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
			
			setEntityAnimation(self, STAND);
			
			self->action = &returnToStart;
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
	
	if (atTarget())
	{
		self->dirY = 0;
		
		self->dirX = self->face == LEFT ? -self->speed : self->speed;
		
		self->action = &swim;
	}
}
