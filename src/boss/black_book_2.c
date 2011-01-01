/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "../event/script.h"

extern Entity *self;

static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void attackFinished(void);
static void hover(void);

Entity *addBlackBook2(int x, int y, char *name)
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

	if (self->active == TRUE)
	{
		minX = getMapStartX();
		minY = getMapStartY();
		
		self->flags &= ~NO_DRAW;
		
		if (cameraAtMinimum())
		{
			centerMapOnEntity(NULL);

			self->action = &doIntro;

			self->thinkTime = 30;
		}
	}
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setContinuePoint(FALSE, self->name, NULL);
		
		self->targetX = self->startX;
		self->targetY = self->startY;
		
		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		self->thinkTime = 120;

		self->action = &introPause;
	}
}

static void introPause()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		initBossHealthBar();
		
		playDefaultBossMusic();
		
		self->action = &attackFinished;
	}
	
	hover();
}

static void attackFinished()
{
	hover();
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
