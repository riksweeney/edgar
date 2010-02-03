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
#include "../system/error.h"
#include "../collisions.h"

extern Entity *self;

static void touch(Entity *);
static void move(void);
static void init(void);
static void rotateAroundTarget(void);

Entity *addRockContainer(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Rock Container");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &rotateAroundTarget;
	e->action = &init;
	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	float height, circ;

	height = (self->endY - self->startY) * 2;

	circ = PI * 2.0f * self->health;
	/*
	printf("Length is %f\n", circ + height);
	printf("Speed is %f\n", self->speed);

	printf("Time to move is %f\n", (circ + height) / self->speed);
	*/
	if (self->thinkTime > 0)
	{
		self->weight = 0;
	}

	self->action = self->weight == 0 ? &move : &rotateAroundTarget;
}

static void touch(Entity *other)
{
	int bottomBefore;

	if (other->dirY > 0)
	{
		/* Trying to move down */

		if (collision(other->x, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			bottomBefore = other->y + other->h - other->dirY - 1;

			if (abs(bottomBefore - self->y) < self->h - 1)
			{
				/* Place the player as close to the solid tile as possible */

				other->y = self->y;
				other->y -= other->h;

				other->standingOn = self;
				other->dirY = 0;
				other->flags |= ON_GROUND;
			}
		}
	}
}

static void move()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;
	}

	else
	{
		if (self->speed > 0)
		{
			self->dirY = self->mental == 1 ? -self->speed : self->speed;
		}

		else
		{
			self->dirY = self->mental == 1 ? self->speed : -self->speed;
		}

		self->y += self->dirY;

		if (self->y <= self->startY)
		{
			self->y = self->startY;

			self->mental = 2;

			self->endX = self->speed > 0 ? 180 : 0;

			self->dirY *= -1;

			self->weight = 0;

			self->action = &rotateAroundTarget;
		}

		else if (self->y >= self->endY)
		{
			self->y = self->endY;

			self->mental = 1;

			self->endX = self->speed > 0 ? 0 : 180;

			self->dirY *= -1;

			self->weight = 0;

			self->action = &rotateAroundTarget;
		}
	}
}

static void rotateAroundTarget()
{
	float prevX, radians;

	self->endX += self->speed * 0.5;

	self->weight += self->speed * 0.5;

	if (self->endX >= 360)
	{
		self->endX = 0;
	}

	if (fabs(self->weight) >= 180)
	{
		self->endX = self->endX == 0 ? 0 : 180;

		self->weight = 0;

		self->action = &move;
	}

	radians = DEG_TO_RAD(self->endX);

	prevX = self->x;

	self->x = (self->health * cos(radians) - 0 * sin(radians));
	self->y = (self->health * sin(radians) + 0 * cos(radians));

	self->x += self->startX;
	self->y += self->mental == 1 ? self->endY : self->startY;

	self->dirX = self->x - prevX;

	self->dirY = fabs(self->speed);
}
