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
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../inventory.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "key_items.h"
#include "../system/random.h"
#include "../world/explosion.h"
#include "../system/error.h"
#include "../hud.h"

extern Entity *self, player;
extern Game game;

static void init(void);
static void entityWait(void);
static void touch(Entity *);
static void activate(int);
static void beginCountdown(void);
static void sonicBoom(void);

Entity *addSonicBoomPanel(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Sonic Boom Panel");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &touch;
	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	self->head = getEntityByObjectiveName(self->requires);
	
	if (self->head == NULL)
	{
		showErrorAndExit("Sonic Boom Panel cannot find partner %s", self->requires);
	}
	
	self->action = &entityWait;
}

static void entityWait()
{
	checkToMap(self);
}

static void activate(int val)
{
	if (self->active == TRUE)
	{
		if (self->mental < 3)
		{
			if (removeInventoryItemByObjectiveName("Power Cell") == TRUE)
			{
				self->mental++;
				
				setEntityAnimation(self, self->mental);
				
				if (self->mental == 3 && self->mental == self->head->mental)
				{
					self->active = FALSE;
					
					self->head->active = FALSE;
					
					self->action = &beginCountdown;
					
					self->thinkTime = 60;
					
					self->health = 10;
				}
			}
			
			else
			{
				setInfoBoxMessage(120, 255, 255, 255, _("Power Cell is required"));
			}
		}
	}
}

static void touch(Entity *other)
{
	if (self->active == TRUE)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to use the Sonic Boom"));
	}
}

static void beginCountdown()
{
	self->thinkTime--;
	
	setInfoBoxMessage(0, 255, 255, 255, _("Firing Sonic Boom in %d"), self->health);
	
	if (self->thinkTime <= 0)
	{
		self->health--;
		
		if (self->health < 0)
		{
			self->action = &sonicBoom;
		}
		
		self->thinkTime = 60;
	}
}

static void sonicBoom()
{
	activateEntitiesWithRequiredName("SONIC_BOOM", TRUE);
	
	self->mental = 0;
	
	self->head->mental = 0;
	
	setEntityAnimation(self, self->mental);
	
	setEntityAnimation(self->head, self->head->mental);
	
	self->action = &entityWait;
}
