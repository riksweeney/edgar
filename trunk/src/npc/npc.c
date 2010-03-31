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

extern Entity *self;
extern Game game;

static void wait(void);
static void talk(int);
static void touch(Entity *);

Entity *addNPC(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add %s", name);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &wait;

	e->draw = &drawLoopingAnimationToMap;
	e->activate = &talk;
	e->touch = &touch;

	e->type = NPC;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	checkToMap(self);
}

static void talk(int val)
{
	runScript(self->requires);
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && game.showHints == TRUE)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to talk to %s"), _(self->objectiveName));
	}
}
