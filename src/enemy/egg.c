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
#include "../enemy/enemies.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../item/item.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self, player;

static void entityWait(void);
static void hatch(void);

Entity *addEgg(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Egg");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->startX = x;
	e->startY = y;

	e->action = &entityWait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &entityDieNoDrop;
	e->pain = NULL;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = NULL;

	e->creditsAction = &entityWait;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 60 && self->thinkTime > 0)
	{
		if (self->thinkTime % 4 == 0)
		{
			self->x = self->startX + (2 * (self->x < self->startX ? 1 : -1));
		}
	}

	if (self->thinkTime <= 0)
	{
		self->x = self->startX;

		self->action = &hatch;

		self->creditsAction = &hatch;
	}

	checkToMap(self);
}

static void hatch()
{
	Entity *e;

	playSoundToMap("sound/enemy/jumping_slime/hatch", -1, self->x, self->y, 0);

	e = addTemporaryItem(self->name, self->x, self->y, self->face, 0, 0);

	e->dirX = -3;
	e->dirY = -2;
	e->face = RIGHT;

	setEntityAnimation(e, "WALK");

	e = addTemporaryItem(self->name, self->x, self->y, self->face, 0, 0);

	e->dirX = 3;
	e->dirY = -2;
	e->face = RIGHT;

	setEntityAnimation(e, "JUMP");

	e = addEnemy(self->objectiveName, 0, 0);

	if (self->flags & LIMIT_TO_SCREEN)
	{
		e->flags |= LIMIT_TO_SCREEN;
	}

	e->x = self->x + (self->w - e->w) / 2;
	e->y = self->y;

	self->inUse = FALSE;
}
