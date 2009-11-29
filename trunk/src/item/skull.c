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

#include "../graphics/animation.h"
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../graphics/graphics.h"
#include "../game.h"
#include "key_items.h"
#include "../system/error.h"

extern Entity *self;

static void wait(void);
static int draw(void);
static void activate(int);

Entity *addSkull(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Skull");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &wait;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->touch = &entityTouch;
	e->activate = &activate;

	e->draw = &draw;

	setEntityAnimation(e, STAND);

	return e;
}

static int draw()
{
	drawLine(self->startX + self->w / 2, self->startY, self->x + self->w / 2, self->y, 255, 255, 255);

	drawLoopingAnimationToMap();

	return TRUE;
}

static void wait()
{
	self->startX += 5;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->x = self->startX + sin(DEG_TO_RAD(self->startX)) * 8;
}

static void activate(int val)
{
	setEntityAnimation(self, val == 1 ? WALK : STAND);
}
