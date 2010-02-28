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
#include "key_items.h"
#include "../system/error.h"
#include "../hud.h"

extern Entity *self;

static void touch(Entity *);
static void wait(void);
static int draw(void);

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
	e->touch = &touch;
	e->draw = &draw;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	if (self->dirX > 0)
	{
		self->endX = getMapRight(self->x, self->y);
		
		self->box.w = self->endX + self->x;
	}
	
	else if (self->dirX < 0)
	{
		self->endX = getMapLeft(self->x, self->y);
		
		self->box.x = self->endX;
	}
	
	if (self->dirY > 0)
	{
		self->endY = getMapFloor(self->x, self->y);
		
		self->box.h = self->endY + self->y;
	}
	
	else if (self->dirY < 0)
	{
		self->endY = getMapCeiling(self->x, self->y);
		
		self->box.y = self->endY;
	}
}

static int draw()
{
	float x, y;

	x = self->x;
	y = self->y;

	self->x = self->startX;
	self->y = self->startY;

	drawLoopingAnimationToMap();

	if (self->dirY > 0)
	{
		self->y += self->h;

		while (self->y < self->endY)
		{
			drawSpriteToMap();

			self->y += self->h;
		}
	}
	
	else if (self->dirY < 0)
	{
		self->y -= self->h;

		while (self->y > self->endY)
		{
			drawSpriteToMap();

			self->y -= self->h;
		}
	}

	else if (self->dirX > 0)
	{
		self->x += self->w;

		while (self->x < self->endX)
		{
			drawSpriteToMap();

			self->x += self->w;
		}
	}
	
	else if (self->dirX < 0)
	{
		self->x -= self->w;

		while (self->x > self->endX)
		{
			drawSpriteToMap();

			self->x -= self->w;
		}
	}

	self->x = x;
	self->y = y;

	return TRUE;
}

static void touch(Entity *other)
{

}
