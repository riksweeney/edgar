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
#include "../system/error.h"
#include "../hud.h"

extern Entity *self;

static void touch(Entity *);

Entity *addLightBeam(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Light Beam");
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
	checkToMap(self);

	if (self->dirX == 0 && self->dirY == 0)
	{
		self->endX = self->x;
		self->endY = self->y;

		self->x = self->startX;
		self->y = self->startY;
	}
}

static void draw()
{
	float x, y;

	x = self->x;
	y = self->y;

	self->x = self->startX;
	self->y = self->startY;

	drawLoopingAnimationToMap();

	if (self->startX == self->endX)
	{
		self->y += self->h;

		while (self->y < self->endY)
		{
			drawSpriteToMap();

			self->y += self->h;
		}
	}

	else
	{
		self->x += self->w;

		while (self->x < self->endX)
		{
			drawSpriteToMap();

			self->x += self->w;
		}
	}

	self->x = x;
	self->y = y;

	return TRUE;
}

static void touch(Entity *)
{

}
