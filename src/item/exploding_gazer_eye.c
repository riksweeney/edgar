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
#include "../game.h"
#include "../graphics/animation.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void entityWait(void);
static void explodeInit(void);
static void explode(void);

Entity *addExplodingGazerEye(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Exploding Gazer Eye");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &entityWait;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->die = &entityDieNoDrop;
	e->pain = &enemyPain;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	checkToMap(self);

	if ((self->flags & ON_GROUND) || self->standingOn != NULL)
	{
		self->dirX = 0;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, "WALK");

			self->thinkTime = 15;

			self->action = &explodeInit;
		}
	}
}

static void explodeInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 15;

		setEntityAnimation(self, "JUMP");

		self->action = &explode;
	}
}

static void explode()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		playSoundToMap("sound/enemy/gazer/flash", -1, self->x, self->y, 0);

		fadeFromColour(255, 255, 255, 60);

		self->inUse = FALSE;

		setPlayerStunned(300);
	}
}
