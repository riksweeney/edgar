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
#include "../entity.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../geometry.h"
#include "../player.h"
#include "../item/key_items.h"
#include "../system/error.h"

extern Entity *self, player, entity[MAX_ENTITIES];

static void entityWait(void);
static void blowPlayerAway(void);
static void lookForFood(void);
static void eatFood(void);

Entity *addHoover(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Hoover");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &entityWait;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void entityWait()
{
	checkToMap(self);
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}
	
	if (self->mental > 0)
	{
		self->mental--;
		
		if (self->mental <= 0)
		{
			setEntityAnimation(self, STAND);
		}
	}
	
	if (self->active == TRUE && self->thinkTime <= 0)
	{
		self->action = &lookForFood;
		
		self->thinkTime = 30;
	}
}

static void lookForFood()
{
	int i;
	
	/* Attack player first */
	
	if (collision(self->x, self->y, 160, self->h, player.x, player.y, player.w, player.h) == 1)
	{
		self->action = &blowPlayerAway;
		
		return;
	}
	
	/* Look for apples */
	
	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && (entity[i].flags & ON_GROUND) && strcmpignorecase(entity[i].name, "item/apple") == 0)
		{
			if (collision(self->x - 320, self->y, 640, self->h, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == 1)
			{
				self->target = &entity[i];
				
				entity[i].flags |= FLY;
				
				setCustomAction(self->target, &helpless, 600, 0, 0);
				
				self->action = &eatFood;
				
				entity[i].targetX = self->x + (self->w - entity[i].w) / 2;
				entity[i].targetY = self->y + (self->h - entity[i].h) / 2;
				
				calculatePath(entity[i].x, entity[i].y, entity[i].targetX, entity[i].targetY, &self->target->dirX, &self->target->dirY);
				
				self->target->dirX *= 4;
				self->target->dirY *= 4;
				
				faceTarget();
				
				return;
			}
		}
	}
	
	self->action = &entityWait;
	
	self->thinkTime = 30;
}

static void blowPlayerAway()
{
	setPlayerStunned(30);

	player.dirX = 12;
	player.dirY = -8;

	self->action = &entityWait;
}

static void eatFood()
{
	setCustomAction(self->target, &helpless, 600, 0, 0);
	
	self->target->x += self->target->dirX;
	self->target->y += self->target->dirY;
	
	if (fabs(self->target->x - self->target->targetX) <= fabs(self->target->dirX) && fabs(self->target->y - self->target->targetY) <= fabs(self->target->dirY))
	{
		setEntityAnimation(self, CUSTOM_1);
		
		self->mental = 60;
		
		self->target->inUse = FALSE;
		
		self->action = &lookForFood;
		
		self->health--;
	}
}
