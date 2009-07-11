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

#include "headers.h"

#include "graphics/animation.h"
#include "system/properties.h"
#include "entity.h"
#include "collisions.h"

static void projectileMove(void);
static void projectileDie(void);
static void removeProjectile(void);

extern Entity *self;

Entity *addProjectile(char *name, Entity *owner, int x, int y, float dirX, float dirY)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a projectile\n");

		exit(1);
	}

	loadProperties(name, e);

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
		printf("No valid thinktime defined for %s\n", name);

		exit(1);
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
	self->dirX *= -1;

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
