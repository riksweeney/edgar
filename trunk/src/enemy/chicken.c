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
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../geometry.h"

extern Entity entity[MAX_ENTITIES], *self;

static void lookForFood(void);
static void wander(void);
static void moveToFood(void);
static void finishEating(void);

Entity *addChicken(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Chicken\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &wander;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void lookForFood()
{
	int i, distance, target, newDistance;

	newDistance = distance = target = -1;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == FALSE || strcmpignorecase(entity[i].name, "item/chicken_feed") != 0)
		{
			continue;
		}

		if (entity[i].health > 3)
		{
			newDistance = getDistance(self->x, self->y, entity[i].x, entity[i].y);

			if (newDistance < 320 && (target == -1 || newDistance < distance))
			{
				distance = newDistance;

				target = i;
			}
		}
	}

	if (target != -1)
	{
		self->target = &entity[target];

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

			setEntityAnimation(self, WALK);
		}

		else
		{
			self->dirX = 0;

			setEntityAnimation(self, STAND);
		}

		self->thinkTime = 180 + prand() % 120;
	}

	if (prand() % 2400 == 0)
	{
		playSoundToMap("sound/enemy/chicken/cluck.ogg", -1, self->x, self->y, 0);
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
	if (self->target->health <= 3)
	{
		self->action = &wander;
	}

	else if (abs(self->x + (self->face == RIGHT ? self->w : 0) - self->target->x) > self->speed)
	{
		self->dirX = self->target->x < self->x ? -self->speed : self->speed;

		self->face = (self->dirX < 0 ? LEFT : RIGHT);

		if (self->dirX != 0 && isAtEdge(self) == 1)
		{
			self->dirX = 0;

			setEntityAnimation(self, STAND);
		}

		else
		{
			setEntityAnimation(self, WALK);
		}
	}

	else if (self->target->flags & ON_GROUND)
	{
		if (strcmpignorecase(self->target->name, "item/chicken_feed") == 0)
		{
			self->target->thinkTime = 600;
		}

		self->action = &doNothing;

		setEntityAnimation(self, ATTACK_1);

		self->animationCallback = &finishEating;
	}

	checkToMap(self);
}

static void finishEating()
{
	self->target->health--;

	setEntityAnimation(self, STAND);

	if (self->target->health <= 0)
	{
		self->target->inUse = FALSE;

		self->target = NULL;

		self->action = &lookForFood;

		self->thinkTime = 0;
	}

	else
	{
		if (strcmpignorecase(self->target->name, "item/chicken_feed") == 0)
		{
			self->target->thinkTime = 600;
		}

		self->action = &doNothing;

		setEntityAnimation(self, ATTACK_1);

		self->animationCallback = &finishEating;
	}
}
