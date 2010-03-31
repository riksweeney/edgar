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
#include "../audio/audio.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "../map.h"
#include "../game.h"
#include "../audio/music.h"
#include "../graphics/gib.h"
#include "../event/trigger.h"
#include "../hud.h"
#include "../inventory.h"
#include "../world/target.h"
#include "../system/error.h"
#include "../geometry.h"
#include "../enemy/enemies.h"

extern Entity *self;

static void initialise(void);
static void doIntro(void);
static void summonLeave(void);
static void summonLeaveRemove(void);
static void hover(void);

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

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->die = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	int minX, minY;

	minX = getMapStartX();
	minY = getMapStartY();

	if (self->active == TRUE)
	{
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

		playSoundToMap("sound/common/spell.ogg", BOSS_CHANNEL, self->x, self->y, 0);

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

		playSoundToMap("sound/common/teleport.ogg", BOSS_CHANNEL, self->x, self->y, 0);

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
