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

#include "../audio/audio.h"
#include "../collisions.h"
#include "../entity.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;
extern Game game;

static void entityWait(void);
static int draw(void);
static void activate(int);
static void touch(Entity *);
static void nextLevelPause(void);

Entity *addTeleporter(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Teleporter");
	}

	loadProperties("common/teleporter", e);

	STRNCPY(e->name, name, sizeof(e->name));

	e->touch = &touch;

	e->action = &entityWait;

	e->draw = &draw;

	e->activate = &activate;

	e->x = x;
	e->y = y;

	e->flags |= OBSTACLE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	self->face = RIGHT;

	if (self->active == FALSE)
	{
		setEntityAnimation(self, "STAND");
	}

	else
	{
		setEntityAnimation(self, "WALK");
	}

	checkToMap(self);
}

static int draw()
{
	Entity *e;

	if (drawLoopingAnimationToMap() == TRUE && self->active == TRUE)
	{
		if (prand() % 3 == 0)
		{
			e = addBasicDecoration(self->x + self->w / 2, self->y, "decoration/particle");

			if (e != NULL)
			{
				e->x += (prand() % 16) * (prand() % 2 == 0 ? -1 : 1);

				e->y -= e->h;

				e->thinkTime = 30 + prand() % 60;

				e->dirY = -5 - prand() % 15;

				e->dirY /= 10;

				e->alpha = 128;

				setEntityAnimationByID(e, prand() % 5);
			}
		}
	}

	return TRUE;
}

static void touch(Entity *other)
{
	pushEntity(other);

	if (other->standingOn == self && !(other->flags & TELEPORTING))
	{
		if (self->active == TRUE)
		{
			self->target = other;

			activate(1);
		}

		else if (other->type == PLAYER)
		{
			setInfoBoxMessage(0, 255, 255, 255, _("This teleporter is not active"));
		}
	}
}

static void activate(int val)
{
	if (strlen(self->objectiveName) == 0)
	{
		/* Internal teleport */

		self->target->targetX = self->endX;
		self->target->targetY = self->endY;

		calculatePath(self->target->x, self->target->y, self->target->targetX, self->target->targetY, &self->target->dirX, &self->target->dirY);

		self->target->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		playSoundToMap("sound/common/teleport", (self->target->type == PLAYER ? EDGAR_CHANNEL : -1), self->target->x, self->target->y, 0);
	}

	else if (self->target->type == PLAYER)
	{
		/* External teleport */

		self->target->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		self->thinkTime = 90;

		self->target->dirX = 0;
		self->target->dirY = 0;

		self->action = &nextLevelPause;

		addParticleExplosion(self->target->x + self->target->w / 2, self->target->y + self->target->h / 2);

		playSoundToMap("sound/common/teleport", (self->target->type == PLAYER ? EDGAR_CHANNEL : -1), self->target->x, self->target->y, 0);
	}

	self->target->standingOn = NULL;

	self->target = NULL;
}

static void nextLevelPause()
{
	char mapName[MAX_VALUE_LENGTH];

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		snprintf(mapName, sizeof(mapName), "%s %s", self->objectiveName, self->name);

		setNextLevelFromScript(mapName);

		self->action = &entityWait;
	}

	checkToMap(self);
}
