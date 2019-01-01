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
#include "../custom_actions.h"
#include "../entity.h"
#include "../event/global_trigger.h"
#include "../event/trigger.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void entityWait(void);
static void blowPlayerAway(void);
static void lookForFood(void);
static void eatFood(void);
static void chewFood(void);
static void hooverSleepy(void);
static void hooverSleep(void);
static void init(void);
static void zMove(void);
static void zVanish(void);

Entity *addHoover(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Hoover");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	if (self->health <= 0)
	{
		setEntityAnimation(self, "CUSTOM_4");

		self->action = &hooverSleep;
	}

	else
	{
		self->action = &entityWait;
	}
}

static void entityWait()
{
	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}

	if (self->mental > 0)
	{
		self->mental--;

		if (self->mental <= 0)
		{
			setEntityAnimation(self, "STAND");
		}
	}

	if (self->active == TRUE && self->thinkTime <= 0)
	{
		self->action = &lookForFood;

		self->thinkTime = 30;
	}

	if (self->health <= 0 && self->mental <= 0)
	{
		setEntityAnimation(self, "CUSTOM_3");

		self->thinkTime = 120;

		self->action = &hooverSleepy;
	}

	facePlayer();
}

static void lookForFood()
{
	EntityList *el, *entities;

	entities = getEntities();

	/* Attack player first */

	if (collision(self->x, self->y - 640, 160, self->h + 640, player.x, player.y, player.w, player.h) == 1)
	{
		setEntityAnimation(self, "ATTACK_1");

		self->mental = 60;

		self->action = &blowPlayerAway;

		return;
	}

	/* Look for apples */

	for (el=entities->next;el!=NULL;el=el->next)
	{
		if (el->entity->inUse == TRUE && (el->entity->flags & ON_GROUND) && strcmpignorecase(el->entity->name, "item/apple") == 0)
		{
			if (collision(self->x - 320, self->y, 640, self->h, el->entity->x, el->entity->y, el->entity->w, el->entity->h) == 1)
			{
				self->target = el->entity;

				faceTarget();

				el->entity->flags |= FLY;

				setCustomAction(self->target, &helpless, 600, 0, 0);

				self->action = &eatFood;

				el->entity->targetX = self->x + self->w;
				el->entity->targetY = self->y + (self->h - el->entity->h) / 2;

				calculatePath(el->entity->x, el->entity->y, el->entity->targetX, el->entity->targetY, &self->target->dirX, &self->target->dirY);

				self->target->dirX *= 4;
				self->target->dirY *= 4;

				setEntityAnimation(self, "ATTACK_2");

				return;
			}
		}
	}

	self->action = &entityWait;

	self->thinkTime = 30;
}

static void blowPlayerAway()
{
	setPlayerStunned(30);

	player.dirX = 12;
	player.dirY = -8;

	self->action = &entityWait;
}

static void eatFood()
{
	setCustomAction(self->target, &helpless, 600, 0, 0);

	self->target->x += self->target->dirX;
	self->target->y += self->target->dirY;

	if (fabs(self->target->x - self->target->targetX) <= fabs(self->target->dirX) && fabs(self->target->y - self->target->targetY) <= fabs(self->target->dirY))
	{
		playSoundToMap("sound/enemy/whirlwind/suck", -1, self->x, self->y, 0);

		setEntityAnimation(self, "CUSTOM_1");

		self->animationCallback = &chewFood;

		self->target->inUse = FALSE;

		self->action = &lookForFood;
	}
}

static void chewFood()
{
	self->mental = 60;

	setEntityAnimation(self, "CUSTOM_2");

	self->health--;
}

static void hooverSleepy()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		fireTrigger(self->objectiveName);

		fireGlobalTrigger(self->objectiveName);

		setEntityAnimation(self, "CUSTOM_4");

		self->action = &hooverSleep;
	}

	checkToMap(self);
}

static void hooverSleep()
{
	Entity *e;

	if (prand() % 90 == 0)
	{
		e = addBasicDecoration(self->x + self->w, self->y, "decoration/z");

		if (e != NULL)
		{
			if (self->face == LEFT)
			{
				e->x = self->x + self->w - e->w - self->offsetX;
			}

			else
			{
				e->x = self->x + self->offsetX;
			}

			e->y = self->y + self->offsetY;

			e->face = RIGHT;

			e->startX = e->x;

			e->action = &zMove;
			e->animationCallback = &zVanish;
		}
	}

	checkToMap(self);
}

static void zMove()
{
	self->health++;

	self->x = self->startX + sin(DEG_TO_RAD(self->health)) * 8;

	self->y -= 0.5;
}

static void zVanish()
{
	self->inUse = FALSE;
}
