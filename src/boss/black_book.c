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
#include "../enemy/enemies.h"
#include "../entity.h"
#include "../event/script.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../map.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../world/target.h"

extern Entity *self;

static void initialise(void);
static void doIntro(void);
static void summonLeave(void);
static void summonLeaveRemove(void);
static void hover(void);
static void init(void);
static void activate(int);
static void waitOnShelf(void);
static void shelfTouch(Entity *);

Entity *addBlackBook(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Black Book");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->die = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case 0:
			self->action = &initialise;
		break;

		default:
			self->touch = &shelfTouch;

			self->activate = &activate;

			self->action = &waitOnShelf;
		break;
	}
}

static void waitOnShelf()
{
	checkToMap(self);
}

static void shelfTouch(Entity *other)
{
	setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
}

static void activate(int val)
{
	runScript("black_book");
}

static void initialise()
{
	if (self->active == TRUE)
	{
		self->flags &= ~NO_DRAW;

		if (cameraAtMinimum())
		{
			centerMapOnEntity(NULL);

			self->action = &doIntro;

			self->thinkTime = 30;
		}
	}

	hover();
}

static void doIntro()
{
	Entity *e;
	Target *t;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= DO_NOT_PERSIST;

		e = addEnemy("boss/mataeus", self->x, self->y);

		t = getTargetByName("MATAEUS_RIGHT_TARGET");

		if (t == NULL)
		{
			showErrorAndExit("Mataeus cannot find target");
		}

		e->targetX = t->x;
		e->targetY = t->y;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

		self->thinkTime = 120;

		self->action = &summonLeave;
	}
}

static void summonLeave()
{
	Target *t;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		t = getTargetByName("BLACK_BOOK_LEAVE_TARGET");

		if (t == NULL)
		{
			showErrorAndExit("Black Book cannot find target");
		}

		self->targetX = t->x;
		self->targetY = t->y;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		playSoundToMap("sound/common/teleport", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &summonLeaveRemove;
	}
}

static void summonLeaveRemove()
{
	self->inUse = FALSE;
}

static void hover()
{
	self->startX++;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 8;
}
