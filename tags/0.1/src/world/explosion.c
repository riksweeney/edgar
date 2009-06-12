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

extern Entity *self;

static void finish(void);
static void explode(void);

Entity *addExplosion(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add an Explosion\n");

		exit(1);
	}

	loadProperties("common/explosion", e);

	e->x = x;
	e->y = y;

	e->type = PROJECTILE;

	e->flags |= FLY|ATTACKING;

	e->face = RIGHT;

	e->action = &explode;
	e->touch = &entityTouch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->animationCallback = &finish;

	return e;
}

static void explode()
{
	playSound("sound/common/explosion.ogg", -1, self->x, self->y, 0);

	self->action = &doNothing;
}

static void finish()
{
	self->inUse = FALSE;
}
