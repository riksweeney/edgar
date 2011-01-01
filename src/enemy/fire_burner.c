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
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../system/error.h"

extern Entity *self, player;

static void entityWait(void);
static void init(void);
static void burn(void);
static void burnStart(void);
static void burnWait(void);

Entity *addFireBurner(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Fire Burner");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;
	
	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	if (self->health == 1)
	{
		self->action = &burn;
	}
	
	else
	{
		self->action = &entityWait;
	}
}

static void burnStart()
{
	self->health = 1;
	
	if (self->startY == -1)
	{
		self->startX = playSoundToMap("sound/enemy/fire_burner/flame.ogg", -1, self->x, self->y, 0);
	}
	
	self->thinkTime = self->maxThinkTime;
	
	setEntityAnimation(self, WALK);
	
	self->animationCallback = &burn;
	
	self->touch = &entityTouch;
	
	self->action = &burnWait;
}

static void burn()
{
	self->action = &burn;
	
	setEntityAnimation(self, JUMP);
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		if (self->startY == -1)
		{
			stopSound(self->startX);
		}
	
		self->frameSpeed = -1;
		
		setEntityAnimation(self, WALK);
		
		self->thinkTime = self->maxThinkTime;
		
		self->animationCallback = &entityWait;
		
		self->action = &burnWait;
	}
}

static void burnWait()
{

}

static void entityWait()
{
	self->frameSpeed = 1;
	
	self->action = &entityWait;
	
	setEntityAnimation(self, STAND);
	
	self->thinkTime--;
	
	self->touch = NULL;
	
	if (self->thinkTime <= 0)
	{
		if (self->active == TRUE)
		{
			self->action = &burnStart;
		}
		
		else
		{
			self->thinkTime = self->maxThinkTime;
		}
	}
}
