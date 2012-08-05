/*
Copyright (C) 2009-2012 Parallel Realities

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
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include "../headers.h"

#include "../collisions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../graphics/graphics.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void touch(Entity *);
static void entityWait(void);
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

	e->action = &entityWait;
	e->touch = &touch;
	e->draw = &draw;

	e->face = RIGHT;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	if (self->dirX > 0)
	{
		self->endX = getMapRight(self->startX, self->startY);

		self->box.w = self->endX - self->x;
	}

	else if (self->dirX < 0)
	{
		self->x = getMapLeft(self->startX, self->startY);

		self->box.w = self->startX - self->x;
	}

	if (self->dirY > 0)
	{
		self->endY = getMapFloor(self->startX, self->startY);

		self->box.h = self->endY - self->y;
	}

	else if (self->dirY < 0)
	{
		self->y = getMapCeiling(self->startX, self->startY);

		self->box.h = self->startY - self->y;
	}
}

static int draw()
{
	if (self->dirX > 0)
	{
		drawBoxToMap(self->x, self->y, self->box.w, 4, 220, 220, 220);
	}

	else if (self->dirX < 0)
	{
		drawBoxToMap(self->x, self->y, self->box.w, 4, 220, 220, 220);
	}

	else if (self->dirY > 0)
	{
		drawBoxToMap(self->x, self->y, 4, self->box.h, 220, 220, 220);
	}

	else if (self->dirY < 0)
	{
		drawBoxToMap(self->x, self->y, 4, self->box.h, 220, 220, 220);
	}

	return TRUE;
}

static void touch(Entity *other)
{
	if (other->type != KEY_ITEM && other->type != PLAYER && other->type != ENEMY
	&& other->type != AUTO_LIFT && other->type != MANUAL_LIFT && other->type != WEAPON
	&& other->type != PROJECTILE)
	{
		if (self->dirX > 0)
		{
			self->endX = other->x;

			self->box.w = self->endX - self->x;
		}

		else if (self->dirX < 0)
		{
			self->x = other->x + other->w;

			self->box.w = self->startX - self->x;
		}

		if (self->dirY > 0)
		{
			self->endY = other->y;

			self->box.h = self->endY - self->y;
		}

		else if (self->dirY < 0)
		{
			self->y = other->y + other->h;

			self->box.h = self->startY - self->y;
		}
	}
}
