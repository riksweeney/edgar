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
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void lookForFood(void);
static void wander(void);
static void moveToFood(void);
static void finishEating(void);
static void creditsMove(void);
static void resumeNormalFunction(void);

Entity *addChicken(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Chicken");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &wander;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->resumeNormalFunction = &resumeNormalFunction;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void lookForFood()
{
	int distance, newDistance;
	EntityList *el, *entities;

	entities = getEntities();

	newDistance = distance = -1;

	self->target = NULL;

	self->mental = 0;

	for (el=entities->next;el!=NULL;el=el->next)
	{
		if (el->entity->inUse == FALSE || strcmpignorecase(el->entity->name, "item/chicken_feed") != 0)
		{
			continue;
		}

		if (el->entity->health > 3)
		{
			newDistance = getDistance(self->x, self->y, el->entity->x, el->entity->y);

			if (newDistance < 320 && (self->target == NULL || newDistance < distance))
			{
				distance = newDistance;

				self->target = el->entity;
			}
		}
	}

	if (self->target != NULL)
	{
		self->face = (self->target->x < self->x ? LEFT : RIGHT);

		self->action = &moveToFood;

		return;
	}

	self->action = &wander;
}

static void wander()
{
	float dirX;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->dirX == 0)
		{
			self->dirX = self->speed * (prand() % 2 == 0 ? -1 : 1);

			setEntityAnimation(self, "WALK");
		}

		else
		{
			self->dirX = 0;

			setEntityAnimation(self, "STAND");
		}

		self->thinkTime = 180 + prand() % 120;
	}

	if (prand() % 2400 == 0)
	{
		playSoundToMap("sound/enemy/chicken/cluck", -1, self->x, self->y, 0);
	}

	if (prand() % 30 == 0)
	{
		lookForFood();
	}

	if (self->dirX < 0)
	{
		self->face = LEFT;
	}

	else if (self->dirX > 0)
	{
		self->face = RIGHT;
	}

	dirX = self->dirX;

	if (self->dirX != 0 && isAtEdge(self) == 1)
	{
		self->dirX = 0;
	}

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->face = (self->face == RIGHT ? LEFT : RIGHT);

		self->dirX = -dirX;
	}
}

static void moveToFood()
{
	if (self->target->health <= 3 || self->target->inUse == FALSE)
	{
		self->target = NULL;

		self->action = &wander;
	}

	else if (abs(self->x + (self->face == RIGHT ? self->w : 0) - self->target->x) > self->speed)
	{
		self->dirX = self->target->x < self->x ? -self->speed : self->speed;

		self->face = (self->dirX < 0 ? LEFT : RIGHT);

		if (self->dirX != 0 && isAtEdge(self) == 1)
		{
			self->dirX = 0;

			setEntityAnimation(self, "STAND");
		}

		else
		{
			setEntityAnimation(self, "WALK");
		}
	}

	else if (self->target->flags & ON_GROUND)
	{
		if (strcmpignorecase(self->target->name, "item/chicken_feed") == 0)
		{
			self->target->thinkTime = 600;
		}

		self->mental = 1;

		self->action = &doNothing;

		setEntityAnimation(self, "ATTACK_1");

		self->animationCallback = &finishEating;
	}

	checkToMap(self);
}

static void finishEating()
{
	self->target->health--;

	setEntityAnimation(self, "STAND");

	if (self->target->health <= 0)
	{
		self->target->inUse = FALSE;

		self->target = NULL;

		self->action = &lookForFood;

		self->thinkTime = 0;

		self->mental = 0;
	}

	else
	{
		if (strcmpignorecase(self->target->name, "item/chicken_feed") == 0)
		{
			self->target->thinkTime = 600;

			self->target->flags &= ~NO_DRAW;
		}

		self->action = &doNothing;

		setEntityAnimation(self, "ATTACK_1");

		self->animationCallback = &finishEating;
	}
}

static void creditsMove()
{
	setEntityAnimation(self, "WALK");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}

static void resumeNormalFunction()
{
	self->action = &wander;
}
