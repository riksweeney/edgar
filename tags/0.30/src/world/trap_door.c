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
#include "../system/properties.h"
#include "../entity.h"
#include "../hud.h"

extern Entity *self;
extern Game game;

static void wait(void);
static void touch(Entity *);

Entity *addTrapDoor(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Trap Door\n");

		exit(1);
	}

	loadProperties(name, e);

	e->touch = &touch;

	e->action = &wait;

	e->draw = &drawLoopingAnimationToMap;

	e->x = x;
	e->y = y;

	e->maxHealth = e->health;

	e->flags |= OBSTACLE;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	self->touch = self->active == FALSE ? &touch : NULL;

	self->action = &wait;

	setEntityAnimation(self, self->active == FALSE ? STAND : WALK);
}

static void touch(Entity *other)
{
	pushEntity(other);
}
