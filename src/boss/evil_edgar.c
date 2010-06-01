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
#include "../system/random.h"
#include "../audio/audio.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../graphics/decoration.h"
#include "../game.h"
#include "../player.h"
#include "../graphics/gib.h"
#include "../system/error.h"

extern Entity *self, player;

static void init(void);
static void fadeIn(void);
static void wait(void);

Entity *addEvilEdgar(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Evil Edgar");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->die = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case 0:
			self->alpha = 0;
			self->action = &fadeIn;
		break;
		
		default:
			self->action = &wait;
		break;
	}
}

static void fadeIn()
{
	if (self->active == TRUE)
	{
		self->flags &= ~NO_DRAW;
		
		if (self->mental == 0)
		{
			self->thinkTime--;
			
			if (self->thinkTime <= 0)
			{
				self->alpha++;
				
				self->thinkTime = 1;
				
				if (self->alpha >= 255)
				{
					self->alpha = 255;
					
					self->mental = 1;
				}
			}
		}
	}
	
	checkToMap(self);
}

static void wait()
{
	if (self->active == TRUE)
	{
		self->flags &= ~NO_DRAW;
		
		self->touch = &entityTouch;
	}
	
	checkToMap(self);
}
