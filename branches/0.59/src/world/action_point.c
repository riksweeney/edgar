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
#include "../system/random.h"
#include "../collisions.h"
#include "../audio/audio.h"
#include "../event/script.h"
#include "../hud.h"
#include "../system/error.h"

extern Game game;
extern Entity *self;

static void wait(void);
static void touch(Entity *);
static void activate(int);

Entity *addActionPoint(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Action Point");
	}

	loadProperties(name, e);

	e->touch = &touch;

	e->action = &wait;

	e->draw = &drawLoopingAnimationToMap;

	e->activate = &activate;

	e->x = x;
	e->y = y;

	e->health = 0;

	e->maxHealth = e->health;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	if (self->active == TRUE)
	{
		self->flags &= ~NO_DRAW;
	}

	else
	{
		self->flags |= NO_DRAW;
	}

	self->thinkTime++;

	self->face = RIGHT;

	self->dirY = 0.2 * cos(DEG_TO_RAD(self->thinkTime));

	self->y += self->dirY;
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && game.showHints == TRUE && self->active == TRUE)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
	}
}

static void activate(int val)
{
	if (self->active == TRUE)
	{
		runScript(self->requires);
	}
}
