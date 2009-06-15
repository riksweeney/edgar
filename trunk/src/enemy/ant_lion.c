/*
Copyright (C) 2009 Parallel Realities

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
#include "../audio/audio.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../decoration.h"
#include "../game.h"

extern Entity *self, player, entity[MAX_ENTITIES];

static void patrol(void);
static void lookForFood(void);
static void moveToTarget(void);
static void hunt(void);
static void trapTarget(void);
static void init(void);
static void leave(void);
static void addDust(void);

Entity *addAntLion(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add the Ant Lion\n");

		exit(1);
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

	setEntityAnimation(e, STAND);

	return e;
}

static void patrol()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->targetX = self->startX + (prand() % (int)(self->endX - self->startX - self->w - 1));

		self->dirX = self->targetX < self->x ? -self->speed / 2 : self->speed / 2;

		self->action = &moveToTarget;
	}

	if (prand() % 30 == 0)
	{
		lookForFood();
	}

	addDust();
}

static void lookForFood()
{
	int i;

	printf("Looking for player\n");

	if (collision(self->x - 320, self->y, 640, self->h, player.x, player.y, player.w, player.h) == 1)
	{
		printf("Spotted player\n");

		self->target = &player;

		/*playSound("sound/boss/ant_lion/earthquake.ogg", BOSS_CHANNEL, self->x, self->y, -1);*/

		self->action = &hunt;
	}

	else
	{
		for (i=0;i<MAX_ENTITIES;i++)
		{
			if (entity[i].inUse == TRUE && strcmpignorecase(entity[i].name, "enemy/grub") == 0 &&
				collision(self->x - 320, self->y, 640, self->h, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == 1)
			{
				self->target = &entity[i];

				/*playSound("sound/boss/ant_lion/earthquake.ogg", BOSS_CHANNEL, self->x, self->y, -1);*/

				self->action = &hunt;

				break;
			}
		}
	}
}

static void moveToTarget()
{
	checkToMap(self);

	if (fabs(self->targetX - self->x) < fabs(self->dirX))
	{
		self->thinkTime = 30 + (prand() % self->maxThinkTime);

		self->action = &patrol;
	}

	addDust();
}

static void hunt()
{
	self->targetX = self->target->x - self->w / 2 + self->target->w / 2;

	/* Position under the player */

	if (abs(self->x - self->targetX) <= self->speed)
	{
		self->dirX = 0;
	}

	else
	{
		self->dirX = self->targetX < self->x ? -self->target->speed * 1.5 : self->target->speed * 1.5;
	}

	checkToMap(self);

	if (self->x < self->startX)
	{
		self->x = self->startX;
	}

	else if (self->x > (self->endX - self->w - 1))
	{
		self->x = self->endX - self->w - 1;
	}

	shakeScreen(LIGHT, 5);

	if (abs(self->x - self->targetX) > 640)
	{
		/* Give up hunting */

		self->action = &patrol;

		stopSound(BOSS_CHANNEL);

		return;
	}

	if (self->target->flags & ON_GROUND)
	{
		if (abs(self->x - self->targetX) <= self->speed)
		{
			/* Trapping */

			trapTarget();
		}
	}

	addDust();
}

static void trapTarget()
{
	Entity *temp;

	/*playSound("sound/boss/ant_lion/eat.ogg", BOSS_CHANNEL, self->x, self->y, 0);*/
	
	playSound("sound/boss/ant_lion/earthquake.ogg", BOSS_CHANNEL, self->x, self->y, -1);

	self->target->flags |= NO_DRAW;

	temp = self;

	self = self->target;

	if (self->type != PLAYER)
	{
		self->inUse = FALSE;
	}

	else
	{
		self->die();
	}

	self = temp;

	self->health = 1;

	self->action = &leave;
}

static void leave()
{
	self->flags |= NO_DRAW;

	self->touch = NULL;
}

static void init()
{
	self->action = self->health == 1 ? &leave : &patrol;

	self->action();
}

static void addDust()
{
	addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
}
