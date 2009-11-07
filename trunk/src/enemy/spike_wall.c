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
#include "../audio/audio.h"
#include "../entity.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../collisions.h"
#include "../geometry.h"
#include "../item/key_items.h"
#include "../system/error.h"

extern Entity *self;

static void move(void);
static void touch(Entity *other);

Entity *addSpikeWall(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Spike Wall");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &move;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->touch = &touch;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void move()
{
	float dirX;
	
	if (self->active == TRUE || self->health == 1)
	{
		dirX = self->dirX;
		
		self->health = 1;
		
		checkToMap(self);
		
		if (self->dirX == 0)
		{
			self->dirX = (dirX > 0 ? -self->speed : self->speed);
		}
	}
	
	else
	{
		checkToMap(self);
	}
}

static void touch(Entity *other)
{
	if (self->active == TRUE || self->health == 1)
	{
		entityTouch(other);
	}
	
	else
	{
		pushEntity(other);
	}
}
