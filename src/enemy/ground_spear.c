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
#include "../collisions.h"
#include "../system/error.h"
#include "../audio/audio.h"

extern Entity *self;

static void wait(void);
static void rise(void);
static void sink(void);
static void init(void);

Entity *addGroundSpear(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Ground Spear");
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
	switch (self->mental)
	{
		case 1:
			self->action = &sink;
		break;
		
		case 2:
			self->action = &rise;
		break;
		
		default:
			self->action = &wait;
		break;
	}
}

static void wait()
{
	if (self->active == TRUE)
	{
		self->thinkTime--;
		
		if (self->health > 0)
		{
			self->y = self->targetY + cos(DEG_TO_RAD(self->endX)) * 2;
			
			self->health--;
			
			if (self->health <= 0)
			{
				self->y = self->startY;
			}
			
			self->endX += 90;
		}
		
		if (self->thinkTime <= 0)
		{
			if (self->y == self->startY)
			{
				self->action = &sink;
				
				self->mental = 1;
			}
			
			else
			{
				if (self->startX == -1)
				{
					playSoundToMap("sound/enemy/ground_spear/spear.ogg", -1, self->x, self->y, 0);
				}
				
				self->health = 15;
				
				self->endX = 0;
				
				self->targetY = self->startY + 2;
				
				self->action = &rise;
				
				self->mental = 2;
			}
		}
	}
}

static void sink()
{
	if (self->active == TRUE)
	{
		if (self->y < self->endY)
		{
			self->y += self->speed;
		}

		else
		{
			self->y = self->endY;
			
			self->thinkTime = self->maxThinkTime;
			
			self->mental = 0;
			
			self->action = &wait;
		}
	}
}

static void rise()
{
	if (self->active == TRUE)
	{
		if (self->y > self->startY)
		{
			self->y -= self->speed;
		}

		else
		{
			self->y = self->startY;
			
			self->thinkTime = self->maxThinkTime;
			
			self->mental = 0;
			
			self->action = &wait;
			
			#if DEV == 1
			if (strcmpignorecase(self->objectiveName, "TESTER") == 0)
			{
				printf("Loop time %d\n", self->health);
				
				exit(0);
			}
			#endif
		}
	}
}
