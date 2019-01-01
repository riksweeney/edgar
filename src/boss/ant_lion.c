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
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/gib.h"
#include "../hud.h"
#include "../map.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;
extern Game game;

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
static void creditsAction(void);

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

	e->creditsAction = &creditsAction;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void patrol()
{
	self->mental--;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->targetX = self->startX + (prand() % (int)(self->endX - self->startX - self->w - 1));

		self->dirX = self->targetX < self->x ? -self->speed / 2 : self->speed / 2;

		self->action = &moveToTarget;
	}

	if (self->mental <= 0)
	{
		lookForFood();

		self->mental = 30;
	}

	addDust();
}

static void lookForFood()
{
	EntityList *el, *entities;

	entities = getEntities();

	if (collision(self->x - 320, self->y, 640, self->h, player.x, player.y, player.w, player.h) == 1)
	{
		self->target = &player;

		playSoundToMap("sound/boss/ant_lion/earthquake", BOSS_CHANNEL, self->x, self->y, -1);

		self->action = &hunt;
	}

	else
	{
		for (el=entities->next;el!=NULL;el=el->next)
		{
			if (el->entity->inUse == TRUE && strcmpignorecase(el->entity->name, "enemy/grub") == 0 &&
				collision(self->x - 320, self->y, 640, self->h, el->entity->x, el->entity->y, el->entity->w, el->entity->h) == 1)
			{
				self->target = el->entity;

				playSoundToMap("sound/boss/ant_lion/earthquake", BOSS_CHANNEL, self->x, self->y, -1);

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

	if (abs(self->x - self->targetX) <= self->speed * 3)
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

		self->thinkTime--;
	}

	else if (self->x > (self->endX - self->w - 1))
	{
		self->x = self->endX - self->w - 1;

		self->thinkTime--;
	}

	else
	{
		self->thinkTime = 60;
	}

	if (game.status == IN_GAME)
	{
		shakeScreen(LIGHT, 5);
	}

	if (self->thinkTime <= 0)
	{
		/* Give up hunting */

		self->action = &patrol;

		self->creditsAction = &leave;

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

	self->creditsAction = &attack;

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

		playSoundToMap("sound/boss/ant_lion/eat", BOSS_CHANNEL, self->x, self->y, 0);

		temp = self;

		self = self->target;

		if (self->type == PLAYER)
		{
			freeEntityList(playerGib());
		}

		else
		{
			freeEntityList(throwGibs("enemy/grub_gibs", 7));
		}

		self = temp;

		self->action = &entityWait;

		self->creditsAction = &entityWait;

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
		if (self->target != NULL && self->target->type == PLAYER && (prand() % 2 == 0))
		{
			setInfoBoxMessage(300, 255, 255, 255, _("Try luring something else into the Ant Lion's pit..."));
		}

		self->action = &leave;

		self->creditsAction = &leave;
	}
}

static void addDust()
{
	addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
}

static void noTouch(Entity *other)
{

}

static void creditsAction()
{
	if (self->target == NULL)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->target = getEntityByName("enemy/grub");

			self->thinkTime = 15;
		}
	}

	else if (self->target->mental == 1)
	{
		self->startX = getMapStartX();

		self->endX = getMapStartX() + SCREEN_WIDTH;

		playSoundToMap("sound/boss/ant_lion/earthquake", BOSS_CHANNEL, self->x, self->y, -1);

		self->creditsAction = &hunt;
	}
}
