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

static void jump(void);
static void move(void);

Entity *addJumpingFireball(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Fireball");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &jump;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

Entity *addFireball(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Fireball");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &move;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void move()
{
	checkToMap(self);

	if (self->dirX == 0 && self->dirY == 0)
	{
		self->inUse = FALSE;
	}
}

static void jump()
{
	if (self->flags & ON_GROUND)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			playSoundToMap("sound/enemy/fireball/fireball", -1, self->x, self->y, 0);

			self->dirY = -self->speed;

			self->thinkTime = self->maxThinkTime;

			self->health = 5;
		}
	}

	else
	{
		self->health--;

		setEntityAnimation(self, self->dirY >= 0 ? "WALK" : "STAND");
	}

	checkToMap(self);
}
