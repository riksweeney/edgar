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

#include "../audio/music.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../event/script.h"
#include "../game.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self, player, playerShield, playerWeapon;
extern Game game;

static void entityWait(void);
static void touch(Entity *);
static void activate(int);

Entity *addLevelExit(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Level Exit");
	}

	loadProperties("common/level_exit", e);

	e->x = x;
	e->y = y;

	e->endX = x;

	STRNCPY(e->name, name, sizeof(e->name));

	e->action = &entityWait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->activate = &activate;
	e->thinkTime = 60;

	e->type = LEVEL_EXIT;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	self->dirX = (self->face == RIGHT ? 10 : -10);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 60;

		self->x = self->startX;
	}

	else if (self->thinkTime % 20 == 0)
	{
		self->x += self->dirX;
	}

	if (self->active == FALSE)
	{
		self->flags |= NO_DRAW;
	}

	else
	{
		self->flags &= ~NO_DRAW;
	}
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && self->active == TRUE)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to go to %s"), _(self->requires));
	}
}

static void activate(int val)
{
	if (self->active == TRUE)
	{
		if (game.mapExitable > 0)
		{
			runScript("items_missing");

			return;
		}

		player.flags |= HELPLESS;

		setCustomAction(&player, &helpless, 600, 0, 0);
		setCustomAction(&player, &invulnerableNoFlash, 600, 0, 0);

		setEntityAnimation(&player, "STAND");
		setEntityAnimation(&playerWeapon, "STAND");
		setEntityAnimation(&playerShield, "STAND");

		player.dirX = 0;

		setNextLevel(self->name, self->objectiveName);

		setTransition(TRANSITION_OUT, &goToNextMap);

		fadeOutMusic(1000);
	}
}
