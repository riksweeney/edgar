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
#include "../game.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../system/error.h"
#include "../system/random.h"
#include "../enemy/rock.h"

extern Entity *self;

static void entityWait(void);
static void stickInWall(void);
static void init(void);

Entity *addCrossbowBolt(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Crossbow Bolt");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->thinkTime = 0;

	return e;
}

static void init()
{
	self->action = self->mental == 1 ? &stickInWall : &entityWait;

	self->touch = self->mental == 1 ? &pushEntity : NULL;
}

static void entityWait()
{
	int i;
	float dirX;
	Entity *e;
	
	dirX = self->dirX;
	
	if (dirX != 0)
	{
		for (e=self->target;e!=NULL;e=e->target)
		{
			if (e->x > self->startX)
			{
				e->flags &= ~NO_DRAW;
			}
			
			else
			{
				e->flags |= NO_DRAW;
			}
		}
	}
	
	checkToMap(self);
	
	if (self->dirX == 0 && dirX != 0)
	{
		playSoundToMap("sound/common/crumble.ogg", -1, self->x, self->y, 0);
		
		self->mental = 1;
		
		self->touch = &pushEntity;
		
		self->x += TILE_SIZE / 2;
		
		for (e=self->target;e!=NULL;e=e->target)
		{
			e->x += TILE_SIZE / 2;
			
			e->dirX = 0;
			
			e->touch = &pushEntity;
		}
		
		for (i=0;i<6;i++)
		{
			e = addSmallRock(self->x, self->y, "common/small_rock");

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			e->dirX = -(prand() % 4 + 1);
			e->dirY = -(prand() % 3 + 3);
		}
		
		shakeScreen(MEDIUM, 15);
	}
}

static void stickInWall()
{
	self->dirY = 0;
}
