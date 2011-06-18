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
#include "../collisions.h"
#include "../system/random.h"
#include "../graphics/graphics.h"
#include "../player.h"
#include "enemies.h"
#include "../system/error.h"

extern Entity *self;

static void entityWait(void);
static void init(void);

Entity *addEnemyGenerator(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Enemy Generator");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &entityDie;
	e->pain = NULL;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	if (strlen(self->objectiveName) == 0)
	{
		showErrorAndExit("Enemy Generator at %f %f is not set correctly", self->x, self->y);
	}

	self->action = &entityWait;

	self->action();
}

static void entityWait()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		/* Only spawn enemies when the player's close enough */

		if (getDistanceFromPlayer(self) < (SCREEN_WIDTH / 2))
		{
			e = addEnemy(self->objectiveName, self->x, self->y);

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			e->startX = self->startX;
			e->startY = self->startY;

			e->endX = self->endX;
			e->endY = self->endY;

			e->face = self->face;

			self->thinkTime = (rand() % self->maxThinkTime);
		}
	}

	checkToMap(self);
}
