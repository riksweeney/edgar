/*
Copyright (C) 2009-2010 Parallel Realities

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

#include "../audio/audio.h"
#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../system/random.h"
#include "../graphics/graphics.h"
#include "../geometry.h"
#include "../system/error.h"

extern Entity *self;

static void wait(void);
static void init(void);
static void moveToTarget(void);
static void changeRandomTarget(void);

Entity *addFlyingBug(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Flying Bug");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &entityDie;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeRandomTarget;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	self->thinkTime = 0;

	if (self->mental == 0)
	{
		playSoundToMap("sound/enemy/bug/buzz.ogg", -1, self->x, self->y, 0);

		self->mental = 1;
	}

	self->action = &wait;

	self->action();
}

static void wait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		changeRandomTarget();

		self->action = &moveToTarget;
	}
}

static void changeRandomTarget()
{
	self->targetX = self->x + (prand() % 64) * (prand() % 2 == 0 ? -1 : 1);
	self->targetY = self->y + (prand() % 64) * (prand() % 2 == 0 ? -1 : 1);

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= self->speed;
	self->dirY *= self->speed;
}

static void moveToTarget()
{
	float x, y;

	if (self->dirX == 0 || self->dirY == 0)
	{
		changeRandomTarget();
	}

	x = self->dirX;
	y = self->dirY;

	checkToMap(self);

	if (atTarget())
	{
		self->thinkTime = 120 + (prand() % self->maxThinkTime);

		self->action = &wait;
	}

	else if (self->dirX != x || self->dirY != y)
	{
		changeRandomTarget();
	}
}
