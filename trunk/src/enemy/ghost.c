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
#include "../graphics/graphics.h"
#include "../entity.h"
#include "../player.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../geometry.h"
#include "../custom_actions.h"
#include "../projectile.h"
#include "../audio/audio.h"
#include "../system/error.h"
#include "../collisions.h"

extern Entity *self;

static void init(void);
static void move(void);
static void moveToSkeleton(void);
static void resurrect(void);
static void resurrectFinish(void);
static void hover(void);
static void touch(Entity *);
static void creditsMove(void);

Entity *addGhost(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a G-G-G-G-G-Ghost!");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	self->mental = 0;
	
	self->action = &move;
}

static void move()
{
	moveLeftToRight();
	
	hover();
}

static void touch(Entity *other)
{
	if (self->mental == 0 && other->health == 0 && other->thinkTime == 0 &&
		(strcmpignorecase(other->name, "enemy/arrow_skeleton") == 0 ||
		strcmpignorecase(other->name, "enemy/sword_skeleton") == 0))
	{
		self->mental = 1;
		
		self->action = &moveToSkeleton;

		self->target = other;

		self->targetX = other->x + self->w / 2 - other->w / 2;
		
		self->thinkTime = 30;
	}
}

static void moveToSkeleton()
{
	if (fabs(self->targetX - self->x) <= fabs(self->dirX))
	{
		self->dirX = 0;

		self->thinkTime = 30;

		self->action = &resurrect;
	}
	
	else
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->dirX = 0;
			
			self->x = self->targetX;
			
			self->action = &resurrect;
		}
	}

	checkToMap(self);

	hover();
}

static void resurrect()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->target->health = self->target->maxHealth;

		self->thinkTime = 60;

		self->action = &resurrectFinish;
	}

	checkToMap(self);

	hover();
}

static void resurrectFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->action = &move;
		
		self->mental = 0;
	}

	checkToMap(self);

	hover();
}

static void hover()
{
	self->startX += 4;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 4;
}

static void creditsMove()
{
	self->face = RIGHT;

	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}

	hover();
}
