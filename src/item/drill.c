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

#include "../system/error.h"

Entity *addDrill(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Drill");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->die = &die;
	e->pain = NULL;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = NULL;

	e->type = ITEM;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	self->animSpeed = self->active == TRUE ? 1 : 0;
}

static void activate(int val)
{
	if (self->health == 0)
	{
		self->health = 1;

		self->thinkTime = 120;

		self->action = &attack;

		self->flags |= ATTACKING;
	}
}

static void touch(Entity *other)
{

}

static void attack()
{
	if (self->y < self->endY)
	{
		self->y += self->speed;
	}

	else
	{
		self->y = self->endY;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &reset;
		}
	}
}

static void reset()
{
	if (self->y > self->startX)
	{
		self->y -= self->speed;
	}

	else
	{
		self->y = self->startX;

		init();
	}
}

static void init()
{
	self->health = 0;
	self->thinkTime = 0;

	self->action = &wait;

	self->flags &= ~ATTACKING;
}
