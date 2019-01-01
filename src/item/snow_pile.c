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
#include "../system/random.h"
#include "item.h"

extern Entity *self, player;

static void entityWait(void);
static void die(void);

Entity *addSnowPile(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Snow Pile");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &entityWait;
	e->die = &die;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	self->face = player.face;

	self->x = player.x + player.w / 2 - self->w / 2;
	self->y = player.y;
}

static void die()
{
	int i;
	Entity *e;

	self->targetX = playSoundToMap("sound/enemy/giant_snowball/crumble", -1, self->x, self->y, 0);

	for (i=0;i<32;i++)
	{
		e = addTemporaryItem("misc/giant_snowball_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += prand() % self->w;
		e->y += prand() % self->h;

		e->dirX = (prand() % 4) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = -2.5;

		e->thinkTime = 60 + (prand() % 120);
	}

	self->inUse = FALSE;
}
