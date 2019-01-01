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

#include "../audio/audio.h"
#include "../collisions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void die(void);
static void hover(void);
static void init(void);

Entity *addBalloon(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Balloon");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->die = &die;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	Entity *e = getEntityByObjectiveName(self->requires);

	if (e == NULL)
	{
		showErrorAndExit("Balloon couldn't find target %s", self->requires);
	}

	self->face = RIGHT;

	self->target = e;

	self->target->flags |= HELPLESS;

	self->action = &hover;

	setEntityAnimationByID(self, self->mental);
}

static void hover()
{
	self->thinkTime += 2;

	if (self->thinkTime >= 360)
	{
		self->thinkTime = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->thinkTime)) * 4;

	self->target->x = self->x + self->w / 2 - self->target->w / 2;

	self->target->y = self->y + self->h;

	self->target->flags |= HELPLESS|FLY;

	self->target->dirY = 0;
}

static void die()
{
	playSoundToMap("sound/item/burst", -1, self->x, self->y, 0);

	self->target->dirY = 0;

	self->target->flags &= ~(HELPLESS|FLY);

	self->inUse = FALSE;
}
