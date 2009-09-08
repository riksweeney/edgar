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
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"

extern Entity *self;

static void walk(void);
static void wait(void);
static void changeFaceStart(void);
static void changeFace(void);
static void changeFaceFinish(void);
static void changeHead(void);

Entity *addTwoHeadedTortoise(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Two-Headed Tortoise\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &walk;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &entityDie;
	e->takeDamage = &entityTakeDamageFlinch;
	e->reactToBlock = &changeDirection;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void walk()
{
	moveLeftToRight();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirX = 0;

		if (prand() % 2 == 0)
		{
			self->action = &changeHead;
			self->action = &changeFaceStart;
		}

		else
		{
			self->action = &changeFaceStart;
		}
	}
}

static void changeFaceStart()
{
	setEntityAnimation(self, CUSTOM_1);

	self->action = &wait;

	self->animationCallback = &changeFace;
	
	self->thinkTime = 60;
}

static void changeFace()
{
	self->thinkTime--;
	
	self->action = &changeFace;
	
	setEntityAnimation(self, CUSTOM_3);
	
	if (self->thinkTime <= 0)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;
	
		self->frameSpeed = -1;
	
		setEntityAnimation(self, CUSTOM_1);
	
		self->animationCallback = &changeFaceFinish;
		
		self->action = &wait;
	}
}

static void changeFaceFinish()
{
	self->frameSpeed = 1;

	setEntityAnimation(self, STAND);

	self->action = &walk;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;
	
	self->thinkTime = 180 + prand() % 420;
}

static void wait()
{
	checkToMap(self);
}

static void changeHead()
{

}
