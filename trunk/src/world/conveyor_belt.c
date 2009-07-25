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

#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/properties.h"

extern Entity *self;

static void wait(void);

Entity *addConveyorBelt(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a conveyor belt\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &wait;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &pushEntity;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	if (self->active != self->dirY)
	{
		if (self->active == FALSE && self->health == 0)
		{
			self->dirX = 0;
		}

		else
		{
			if (self->active == TRUE)
			{
				self->frameSpeed = self->face == LEFT ? -abs(self->frameSpeed) : self->frameSpeed;

				self->dirX = self->face == LEFT ? -fabs(self->speed) : self->speed;
			}

			else
			{
				self->frameSpeed = self->face == RIGHT ? -abs(self->frameSpeed) : self->frameSpeed;

				self->dirX = self->face == RIGHT ? -fabs(self->speed) : self->speed;
			}
		}

		self->dirY = self->active;
	}
}
