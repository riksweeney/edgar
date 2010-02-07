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
#include "../system/error.h"
#include "../system/random.h"
#include "../collisions.h"
#include "../graphics/decoration.h"
#include "../audio/audio.h"

extern Entity *self;

static void crush(void);

Entity *addCrusher(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Crusher");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &crush;
	e->touch = &entityTouch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void crush()
{
	if (self->thinkTime > 0)
	{
		if (self->active == TRUE || !(self->active == FALSE && self->y == self->startY))
		{
			self->thinkTime--;
		}
	}

	else
	{
		self->y += self->mental == 1 ? self->speed : -self->speed;

		if (self->y >= self->endY)
		{
			self->y = self->endY;

			self->mental = 2;
			
			self->thinkTime = 30;
			
			if (self->health == -1)
			{
				playSoundToMap("sound/common/crunch.ogg", -1, self->x, self->y, 0);
			}
		}
		
		else if (self->y <= self->startY)
		{
			self->y = self->startY;

			self->mental = 1;
			
			self->thinkTime = self->maxThinkTime;
		}
	}
}
