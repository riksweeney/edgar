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
#include "../entity.h"
#include "../enemy/enemies.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../collisions.h"
#include "../geometry.h"
#include "../system/error.h"

static void init(void);
static void lookForPlayer(void);
static void followPlayer(void);
static void summonEnemies(void);

extern Entity *self, player;

Entity *addScanner(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Scanner");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	self->endY = getMapFloor(self->x, self->y) - self->y;

	self->action = &lookForPlayer;
}

static void lookForPlayer()
{
	moveLeftToRight();

	if (player.alpha == 255 && collision(self->x, self->y, self->h, self->endY, player.x, player.y, player.w, player.h) == 1)
	{
		self->thinkTime = 300;

		activateEntitiesWithRequiredName(self->objectiveName, FALSE);

		if (self->mental == 1)
		{
			summonEnemies();
		}

		setEntityAnimation(self, WALK);

		self->action = &followPlayer;
	}
}

static void followPlayer()
{
	self->targetX = self->target->x - self->w / 2 + self->target->w / 2;

	/* Position under the player */

	if (abs(self->x - self->targetX) <= self->speed)
	{
		self->dirX = 0;
	}

	else
	{
		self->dirX = self->targetX < self->x ? -self->target->speed * 3 : self->target->speed * 3;
	}

	checkToMap(self);

	if (collision(self->x, self->y, self->h, self->endY, player.x, player.y, player.w, player.h) == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			activateEntitiesWithRequiredName(self->objectiveName, TRUE);

			setEntityAnimation(self, STAND);

			self->action = &lookForPlayer;
		}
	}
}

static void summonEnemies()
{
	char summonList[MAX_VALUE_LENGTH], enemyToSummon[MAX_VALUE_LENGTH];
	char *token;
	int i, summonIndex = 0, summonCount = 0;
	Entity *e;

	for (i=0;i<2;i++)
	{
		STRNCPY(summonList, self->requires, MAX_VALUE_LENGTH);

		token = strtok(summonList, "|");

		while (token != NULL)
		{
			token = strtok(NULL, "|");

			summonCount++;
		}

		if (summonCount == 0)
		{
			showErrorAndExit("Scanner at %f %f has no summon list", self->x, self->y);
		}

		summonIndex = prand() % summonCount;

		STRNCPY(summonList, self->requires, MAX_VALUE_LENGTH);

		summonCount = 0;

		token = strtok(summonList, "|");

		while (token != NULL)
		{
			if (summonCount == summonIndex)
			{
				break;
			}

			token = strtok(NULL, "|");

			summonCount++;
		}

		snprintf(enemyToSummon, MAX_VALUE_LENGTH, "enemy/%s", token);

		e = addEnemy(enemyToSummon, self->x, self->y);

		e->x = self->x;

		e->y = self->y;

		e->targetX = self->x + (i == 0 ? -64 : 64);

		e->targetY = self->y;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING);
	}
}
