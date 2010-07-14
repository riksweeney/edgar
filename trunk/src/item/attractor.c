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
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../geometry.h"
#include "../system/random.h"
#include "../graphics/decoration.h"
#include "../custom_actions.h"
#include "../system/error.h"

extern Entity *self, entity[MAX_ENTITIES], player;

static void entityWait(void);
static void addRiftEnergy(int, int);
static void energyMoveToRift(void);

Entity *addAttractor(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Attractor");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &entityWait;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void entityWait()
{
	int i;
	
	setEntityAnimation(self, self->active == FALSE ? STAND : WALK);
	
	if (self->active == TRUE)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			for (i=0;i<MAX_ENTITIES;i++)
			{
				if (entity[i].inUse == TRUE && entity[i].type == ENEMY &&
					collision(self->x - self->mental, self->y, self->mental * 2, self->mental * 2, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == 1)
				{
					setCustomAction(&entity[i], &attract, self->maxThinkTime, 0, (entity[i].x < (self->x + self->w / 2) ? self->speed : -self->speed));
				}
			}
			
			self->thinkTime = self->maxThinkTime;
			
			if (collision(self->x - self->mental, self->y, self->mental * 2, self->mental * 2, player.x, player.y, player.w, player.h) == 1)
			{
				setCustomAction(&player, &attract, self->maxThinkTime, 0, (player.x < (self->x + self->w / 2) ? self->speed : -self->speed));
			}
		}
		
		if (prand() % 3 == 0)
		{
			addRiftEnergy(self->x + self->w / 2, self->y + self->h / 2);
		}
	}
}

static void addRiftEnergy(int x, int y)
{
	Entity *e;

	e = addBasicDecoration(x, y, "decoration/rift_energy");

	e->x += prand() % 128 * (prand() % 2 == 0 ? -1 : 1);
	e->y += prand() % 128 * (prand() % 2 == 0 ? -1 : 1);

	x -= e->w / 2;
	y -= e->h / 2;

	e->targetX = x;
	e->targetY = y;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->dirX *= 8;
	e->dirY *= 8;

	e->action = &energyMoveToRift;
}

static void energyMoveToRift()
{
	self->x += self->dirX;
	self->y += self->dirY;

	if (atTarget())
	{
		self->inUse = FALSE;
	}
}
