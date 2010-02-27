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

Entity *addMirror(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Mirror");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &doNothing;
	e->touch = &touch;
	e->fallout = &itemFallout;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void touch(Entity *other)
{
	if (strcmpignorecase(other->name, "item/light_beam") == 0)
	{
		other->dirX = 0;
		other->dirY = 0;

		/* Light moving vertically */

		if (other->dirY > 0 && self->mental == 0)
		{
			other->x = self->x + (self->face == LEFT ? 0 : self->w);
			other->y = self->y + self->h / 2 - other->h / 2;
		}

		else if (other->dirY < 0 && self->mental == 1)
		{
			other->x = self->x + (self->face == LEFT ? 0 : self->w);
			other->y = self->y + self->h / 2 - other->h / 2;
		}

		/* Lift moving horizontally */

		else if (other->dirX > 0 && self->face == LEFT)
		{
			other->x = self->x + self->w / 2 - other->w / 2;
			other->y = self->y + (self->mental == 0 ? 0 : self->h);
		}

		else if (other->dirX < 0 && self->face == RIGHT)
		{
			other->x = self->x + (self->face == LEFT ? 0 : self->w);
			other->y = self->y + (self->mental == 0 ? 0 : self->h);
		}
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
