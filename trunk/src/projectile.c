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

#include "headers.h"

#include "audio/audio.h"
#include "graphics/animation.h"
#include "system/properties.h"
#include "entity.h"
#include "collisions.h"
#include "system/random.h"
#include "system/error.h"

static void projectileMove(void);
static void projectileDie(void);
static void removeProjectile(void);
static void stickToTargetWait(void);

extern Entity *self;

Entity *addProjectile(char *name, Entity *owner, int x, int y, float dirX, float dirY)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a projectile");
	}

	loadProperties(name, e);
	
	e->flags &= ~PLAYER_TOUCH_ONLY;

	e->x = x;
	e->y = y;

	e->dirX = dirX;
	e->dirY = dirY;

	e->action = &projectileMove;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &entityTouch;

	e->type = PROJECTILE;

	e->parent = owner;

	if (e->thinkTime == 0)
	{
		showErrorAndExit("No valid thinktime defined for %s", name);
	}

	if (hasEntityAnimation(e, DIE) == TRUE)
	{
		e->die = &projectileDie;
	}

	else
	{
		e->die = &removeProjectile;
	}

	e->face = e->dirX > 0 ? RIGHT : LEFT;

	setEntityAnimation(e, STAND);

	return e;
}

static void projectileMove()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}

	checkToMap(self);
}

void bounceOffShield()
{
	self->dirX = (self->dirX < 0 ? 5 : -5);
	
	self->dirY = -5;

	self->flags &= ~FLY;

	self->touch = NULL;
}

static void projectileDie()
{
	self->damage = 0;

	setEntityAnimation(self, DIE);

	self->touch = NULL;

	self->dirX = self->dirY = 0;

	self->animationCallback = &removeProjectile;
}

static void removeProjectile()
{
	self->inUse = FALSE;
}

void stickToTarget()
{
	if (self->target != NULL)
	{
		self->thinkTime = 600;

		self->action = &stickToTargetWait;

		self->flags |= DO_NOT_PERSIST;

		self->touch = NULL;

		self->startX = prand() % 6;

		self->startY = prand() % 4 * prand() % 2 == 0 ? -1 : 1;
	}

	else
	{
		if (hasEntityAnimation(self, DIE) == TRUE)
		{
			self->die = &projectileDie;
		}

		else
		{
			self->die = &removeProjectile;
		}

		self->die();
	}
}

static void stickToTargetWait()
{
	self->thinkTime--;

	if (self->target == NULL || self->target->inUse == FALSE || self->thinkTime <= 0)
	{
		self->inUse = FALSE;

		return;
	}

	self->x = self->target->x;
	self->y = self->target->y;

	self->face = self->target->face == LEFT ? RIGHT : LEFT;

	self->x += self->face == RIGHT ? -self->w + self->startX : self->target->w - self->targetX;

	self->y += self->startY;
}
