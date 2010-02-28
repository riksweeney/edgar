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
#include "key_items.h"
#include "light_beam.h"
#include "../system/error.h"

extern Entity *self;

static void wait(void);

Entity *addLightSource(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Light Source");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &wait;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	if (self->target == NULL)
	{
		self->target = addLightBeam(0, 0, "item/light_beam");
		
		self->target->x = self->x + self->w / 2 - self->target->w / 2;
		self->target->y = self->y + self->h / 2 - self->target->h / 2;
		
		self->target->dirY = self->target->speed;
		
		self->target->startX = self->target->x;
		self->target->startY = self->target->y;
	}
}
