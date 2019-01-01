/*
Copyright (C) 2009-2019 Parallel Realities

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
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void entityWait(void);
static int draw(void);

Entity *addDrawbridgeAnchor(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Drawbridge Anchor");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &entityWait;

	e->draw = &draw;

	setEntityAnimation(e, "STAND");

	e->thinkTime = 0;

	return e;
}

static void entityWait()
{
	self->layer = self->mental < 0 ? BACKGROUND_LAYER : FOREGROUND_LAYER;

	checkToMap(self);
}

static int draw()
{
	drawLoopingAnimationToMap();

	self->x += self->offsetX;
	self->y -= self->offsetY;

	setEntityAnimation(self, "WALK");

	self->face = LEFT;

	while (self->x < self->endX)
	{
		drawSpriteToMap();

		self->x += self->offsetX;
		self->y -= self->offsetY;
	}

	self->face = RIGHT;

	setEntityAnimation(self, "STAND");

	self->x = self->startX;
	self->y = self->startY;

	return TRUE;
}
