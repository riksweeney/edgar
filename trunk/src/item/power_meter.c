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
#include "../collisions.h"
#include "../system/error.h"
#include "../hud.h"

extern Entity *self;

static void entityWait(void);

Entity *addPowerMeter(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Power Meter");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &entityWait;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void entityWait()
{
	int frameCount;
	
	self->mental--;
	
	if (self->mental <= 0)
	{
		self->mental = 0;
	}
	
	if (self->health != self->mental)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->health += (self->mental < self->health ? -1 : 1);
			
			self->thinkTime = 30;
		}
	}
	
	frameCount = getFrameCount(self);
	
	if (self->health >= frameCount)
	{
		self->health = frameCount - 1;
		
		if (self->endX != 1)
		{
			activateEntitiesWithRequiredName(self->objectiveName, TRUE);
			
			self->endX = 1;
		}
	}
	
	self->currentFrame = self->health;
	
	checkToMap(self);
}
