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
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../graphics/decoration.h"
#include "../game.h"
#include "../player.h"
#include "../graphics/gib.h"
#include "../system/error.h"

extern Entity *self, player, entity[MAX_ENTITIES];

static void patrol(void);
static void lookForFood(void);
static void moveToTarget(void);
static void hunt(void);
static void trapTarget(Entity *);
static void leave(void);
static void addDust(void);
static void entityWait(void);
static void noTouch(Entity *);
static void attack(void);

Entity *addAntLion(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Ant Lion");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &patrol;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &noTouch;
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

	if (collision(self->x - 320, self->y, 640, self->h, player.x, player.y, player.w, player.h) == 1)
	{
		self->target = &player;

		playSoundToMap("sound/boss/ant_lion/earthquake.ogg", BOSS_CHANNEL, self->x, self->y, -1);

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

				playSoundToMap("sound/boss/ant_lion/earthquake.ogg", BOSS_CHANNEL, self->x, self->y, -1);

				self->action = &hunt;

				break;
			}
		}
	}
}

static void moveToTarget()
{
	checkToMap(self);

	if (fabs(self->targetX - self->x) <= fabs(self->dirX))
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

		self->touch = &trapTarget;
	}

	else
	{
		self->dirX = self->targetX < self->x ? -self->target->speed * 3 : self->target->speed * 3;

		self->touch = &noTouch;
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

		self->target = NULL;

		self->touch = &noTouch;

		stopSound(BOSS_CHANNEL);

		return;
	}

	addDust();
}

static void trapTarget(Entity *other)
{
	if (self->target == NULL || other != self->target)
	{
		return;
	}

	if (abs((self->y + self->h) - (other->y + other->h)) > 5)
	{
		return;
	}

	/* Trap the target */

	self->action = &attack;

	self->touch = NULL;

	self->y += self->h;

	self->flags &= ~NO_DRAW;
}

static void attack()
{
	Entity *temp;

	self->y -= 15;

	if (self->y < self->startY)
	{
		self->y = self->startY;

		playSoundToMap("sound/boss/ant_lion/eat.ogg", BOSS_CHANNEL, self->x, self->y, 0);

		temp = self;

		self = self->target;

		if (self->type == PLAYER)
		{
			freeEntityList(playerGib());
			
			if (prand() % 3 == 0)
			{
				setInfoBoxMessage(300, 255, 255, 255, _("Try luring something else into the Ant Lion's pit..."));
			}
		}

		else
		{
			freeEntityList(throwGibs("enemy/grub_gibs", 7));
		}

		self = temp;

		self->target = NULL;

		self->action = &entityWait;

		self->touch = NULL;

		self->thinkTime = 120;

		self->targetY = self->y + self->h;
	}
}

static void leave()
{
	if (self->y < self->targetY)
	{
		self->y += 0.25;
	}

	else
	{
		self->inUse = FALSE;
	}
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &leave;
	}
}

static void addDust()
{
	addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
}

static void noTouch(Entity *other)
{

}
