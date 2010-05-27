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

#include "../audio/audio.h"
#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../system/error.h"

extern Entity *self;
extern Game game;

static void wait(void);
static void touch(Entity *);
static void init(void);

Entity *addHighStriker(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a High Striker");
	}

	loadProperties(name, e);

	e->touch = &touch;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;

	e->x = x;
	e->y = y;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	if (self->health != 14)
	{
		self->health = self->maxHealth = 0;
	}
	
	else
	{
		setEntityAnimation(self, self->health);
	}
	
	self->maxThinkTime = self->thinkTime = 0;

	self->action = &wait;
}

static void wait()
{
	if (self->maxHealth != self->health)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->health += (self->maxThinkTime == 1 ? 1 : -1);
			
			setEntityAnimation(self, self->health);
			
			if (self->health == self->maxHealth || self->health == 0)
			{
				if (self->health == 14)
				{
					activateEntitiesWithRequiredName(self->objectiveName, TRUE);
					
					playSoundToMap("sound/item/striker_top.ogg", -1, self->x, self->y, 0);
				}
				
				else
				{
					self->maxHealth = 0;
					
					self->thinkTime = 90;
					
					if (self->health == 0)
					{
						self->maxThinkTime = 0;
						
						self->thinkTime = 0;
					}
					
					else
					{
						self->maxThinkTime = self->maxThinkTime == 1 ? -1 : 1;
					}
				}
			}
			
			else
			{
				self->thinkTime = 4;
			}
		}
	}
	
	checkToMap(self);
}

static void touch(Entity *other)
{
	float dirY = other->dirY;
	
	pushEntity(other);

	if (other->standingOn == self)
	{
		dirY *= other->weight;
		
		dirY /= 25;
		
		dirY *= 14;
		
		if (abs(dirY) > self->maxHealth && self->maxThinkTime == 0)
		{
			self->maxHealth = dirY > 14 ? 14 : dirY;
			
			self->maxThinkTime = 1;
		}
	}
}
