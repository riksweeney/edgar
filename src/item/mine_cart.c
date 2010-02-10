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
#include "key_items.h"
#include "item.h"
#include "../system/error.h"
#include "../hud.h"

extern Entity *self;

static void touch(Entity *);
static void init(void);
static void resumeNormalFunction(void);

Entity *addMineCart(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Mine Cart");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &touch;
	e->fallout = &itemFallout;
	e->resumeNormalFunction = &resumeNormalFunction;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	setEntityAnimation(self, self->health >= 6 ? WALK : STAND);
	
	self->action = &doNothing;
}

static void resumeNormalFunction()
{
	self->touch = &touch;
}

static void touch(Entity *other)
{
	if (strcmpignorecase(other->name, "common/large_rock") == 0)
	{
		self->health++;
		
		if (self->health == 1)
		{
			setInfoBoxMessage(60, _("Filling mine cart..."));
		}
		
		if (self->health >= 6)
		{
			STRNCPY(self->objectiveName, "Full Mine Cart", sizeof(self->objectiveName));
			
			setEntityAnimation(self, WALK);
		}
		
		other->inUse = FALSE;
	}
	
	else
	{
		if (other->type != PLAYER)
		{
			self->flags |= OBSTACLE;
		}
		
		else
		{
			self->flags &= ~OBSTACLE;
		}
		
		pushEntity(other);
		
		self->flags &= ~OBSTACLE;
	}
}
