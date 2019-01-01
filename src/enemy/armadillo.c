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
#include "../item/item.h"
#include "../item/key_items.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void die(void);
static void creditsMove(void);

Entity *addArmadillo(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Armadillo");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &moveLeftToRight;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void die()
{
	Entity *e;

	if (prand() % 3 == 0)
	{
		e = addKeyItem("item/spike_ball", self->x + self->w / 2, self->y);

		e->x -= e->w / 2;

		e->action = &generalItemAction;

		e->flags |= DO_NOT_PERSIST;
	}

	playSoundToMap("sound/enemy/armadillo/armadillo_die", -1, self->x, self->y, 0);

	entityDie();
}

static void creditsMove()
{
	self->face = RIGHT;

	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}
