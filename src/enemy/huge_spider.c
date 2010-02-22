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
#include "../geometry.h"
#include "../hud.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../system/error.h"

extern Entity *self;

static void moveToTarget(void);
static void wait(void);
static void takeDamage(Entity *, int);
static void shudder(void);
static void touch(Entity *);

Entity *addHugeSpider(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Huge Spider");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &wait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->takeDamage = &takeDamage;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	int x, y;
	
	if (self->thinkTime > 0)
	{
		self->thinkTime--;
	}
	
	else if (self->thinkTime <= 0)
	{
		x = (32 + (prand() % 128)) * (prand() % 2 == 0 ? 1 : -1);
		y = (32 + (prand() % 128)) * (prand() % 2 == 0 ? 1 : -1);
		
		self->targetX = self->x + x;
		self->targetY = self->y + y;
		
		if (self->targetX < self->startX)
		{
			self->targetX = self->startX;
		}
		
		else if (self->targetX > self->endX)
		{
			self->targetX = self->endX;
		}
		
		if (self->targetY < self->startY)
		{
			self->targetY = self->startY;
		}
		
		else if (self->targetY > self->endY)
		{
			self->targetY = self->endY;
		}
		
		self->action = &moveToTarget;
		
		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
		
		self->dirX *= self->speed;
		self->dirY *= self->speed;
		
		self->thinkTime = 120;
	}
}

static void moveToTarget()
{
	checkToMap(self);
	
	self->thinkTime--;
	
	if (atTarget() || self->thinkTime <= 0)
	{
		self->dirX = 0;
		
		self->dirY = 0;
		
		self->thinkTime = 60 + prand() % 180;
		
		self->action = &wait;
	}
}

static void takeDamage(Entity *other, int damage)
{
	setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);

	playSoundToMap("sound/common/dink.ogg", 2, self->x, self->y, 0);
	
	if (prand() % 10 == 0)
	{
		setInfoBoxMessage(60, _("This weapon is not having any effect..."));
	}
}

static void touch(Entity *other)
{
	if (!(self->flags & INVULNERABLE) && other->type == ITEM && strcmpignorecase(other->name, "item/repellent_spray") == 0)
	{
		self->mental++;
		
		printf("%d\n", self->mental);
		
		if (self->mental == 50)
		{
			self->action = &shudder;
			
			self->targetX = self->x;
			
			self->thinkTime = 300;
			
			setInfoBoxMessage(60, _("Now! Run while it's stunned!"));
			
			self->health = 0;
		}
		
		setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);
		setCustomAction(self, &flashWhite, 6, 0, 0);
	}
	
	else
	{
		entityTouch(other);
	}
}

static void shudder()
{
	self->health += 90;

	if (self->health >= 360)
	{
		self->health %= 360;
	}
	
	self->x = self->targetX + sin(DEG_TO_RAD(self->health)) * 8;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->health = 0;
		
		self->x = self->startX;
		
		self->action = &wait;
	}
	
	checkToMap(self);
}
