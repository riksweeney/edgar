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

#include "../collisions.h"
#include "../entity.h"
#include "../game.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;
extern Game game;

static void entityWait(void);
static void touch(Entity *);
static void activate(int);

Entity *addSavePoint(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Save Point");
	}

	loadProperties("common/save_point", e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->activate = &activate;
	e->action = &entityWait;

	e->type = SAVE_POINT;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	int frame = self->currentFrame;

	self->face = RIGHT;

	self->thinkTime--;

	if (self->thinkTime < 0)
	{
		self->thinkTime = 0;

		setEntityAnimation(self, "STAND");

		self->currentFrame = frame;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	int frame = self->currentFrame;

	if (other->type == PLAYER)
	{
		self->thinkTime = 5;

		setEntityAnimation(self, "WALK");

		self->currentFrame = frame;

		if (game.showHints == TRUE)
		{
			setInfoBoxMessage(0, 255, 255, 255, _("Press Action to save your game"));
		}
	}
}

static void activate(int val)
{
	showSaveDialog();
}
