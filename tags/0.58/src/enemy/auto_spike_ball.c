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

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../projectile.h"
#include "../audio/audio.h"
#include "../system/error.h"

extern Entity *self;

static void wait(void);
static void explode(void);

Entity *addAutoSpikeBall(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Auto Spike Ball");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->face = RIGHT;

	e->action = &wait;
	
	e->touch = &entityTouch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->action = &explode;
	}
}

static void explode()
{
	int i;
	Entity *e;

	for (i=0;i<360;i+=12)
	{
		e = addProjectile("weapon/spike", self, 0, 0, 0, 0);
		
		e->damage = 1;

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		e->dirX = (0 * cos(DEG_TO_RAD(i)) - 12 * sin(DEG_TO_RAD(i)));
		e->dirY = (0 * sin(DEG_TO_RAD(i)) + 12 * cos(DEG_TO_RAD(i)));

		e->flags |= FLY;
	}

	self->inUse = FALSE;
}
