/*
Copyright (C) 2009-2013 Parallel Realities

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

#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "item.h"
#include "key_items.h"

extern Entity *self;

static void init(void);

Entity *addPuzzlePiece(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Puzzle Piece");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &keyItemTouch;
	e->fallout = &itemFallout;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	setEntityAnimationByID(self, self->mental);

	self->action = &doNothing;
}
