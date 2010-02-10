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
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../geometry.h"
#include "../event/script.h"
#include "../system/error.h"

extern Entity *self, entity[MAX_ENTITIES];

static void wait(void);
static void init(void);
static void dropRock(void);
static void grabRock(void);
static void moveToTarget(void);
static void moveToStart(void);
static void moveToEnd(void);
static void touch(Entity *);
static void activate(int);
static void addChain(Entity *);
static void chainWait(void);

Entity *addGrabber(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Grabber");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = LEFT;

	e->action = &init;
	e->touch = &touch;
	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	if (self->health != 1 && self->active == TRUE)
	{
		if (self->target == NULL)
		{
			setEntityAnimation(self, WALK);

			self->action = &grabRock;
		}

		else
		{
			self->target->y = self->y + self->h / 2;

			self->target->x = self->x + (self->w - self->target->w) / 2;

			self->thinkTime = 60;

			setEntityAnimation(self, WALK);

			self->action = &dropRock;
		}
	}

	else if (self->health == 1)
	{
		self->action = &moveToEnd;
	}

	checkToMap(self);
}

static void dropRock()
{
	Entity *temp;
	
	if (self->target != NULL)
	{
		self->target->flags &= ~(HELPLESS|FLY);

		self->target->y = self->y + self->h / 2;

		self->target->x = self->x + (self->w - self->target->w) / 2;

		if (self->target->type == ENEMY)
		{
			self->target->type = PROJECTILE;

			self->target->damage = 100;

			self->target->parent = self;
		}
		
		else if (self->target->resumeNormalFunction != NULL)
		{
			temp = self;
			
			self = self->target;
			
			self->resumeNormalFunction();
			
			self = temp;
		}

		else
		{
			self->target->touch = &pushEntity;
		}

		self->target = NULL;
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, STAND);

			moveToStart();
		}
	}
}

static void grabRock()
{
	self->dirY = self->speed;

	if (self->standingOn != NULL)
	{
		touch(self->standingOn);
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		setEntityAnimation(self, STAND);

		moveToStart();
	}
}

static void moveToTarget()
{
	/* Move vertically first */

	if (self->y > self->targetY)
	{
		self->dirY = -self->speed;

		self->dirX = 0;
	}

	/* Now move horizontally */

	else
	{
		self->y = self->targetY;

		self->dirX = self->targetX < self->x ? -self->speed : self->speed;

		self->dirY = 0;
	}

	checkToMap(self);

	if (self->target != NULL)
	{
		self->target->y = self->y + self->h / 2;

		self->target->x = self->x + (self->w - self->target->w) / 2;

		self->target->x += self->dirX;
		self->target->y += self->dirY;
	}

	if (fabs(self->x - self->targetX) <= fabs(self->dirX) && fabs(self->y - self->targetY) <= fabs(self->dirY))
	{
		self->x = self->targetX;
		self->y = self->targetY;

		self->dirX = 0;
		self->dirY = 0;

		if (self->health == 1)
		{
			if (self->x == self->endX && self->y == self->endY)
			{
				setEntityAnimation(self, STAND);

				self->health = 2;

				self->startX = self->endX;
				self->startY = self->endY;
			}
		}

		else if (self->health == -1)
		{
			self->health = 0;
		}

		self->active = FALSE;

		self->action = &wait;
	}
}

static void touch(Entity *other)
{
	Entity *temp;

	if (self->active == TRUE)
	{
		if (strcmpignorecase(other->name, "boss/golem_boss") == 0)
		{
			setEntityAnimation(self, STAND);

			if (other->currentAnim == other->animation[CUSTOM_1] && self->health == 2)
			{
				temp = self;

				self = other;

				self->die();

				self = temp;
			}

			else
			{
				moveToStart();
			}
		}

		else if (((other->flags & PUSHABLE) && (other->flags & ON_GROUND)) || (self->health == -1 && self->target == NULL))
		{
			setEntityAnimation(self, STAND);

			other->flags |= HELPLESS|FLY;

			other->touch = &entityTouch;

			other->damage = 0;

			other->dirX = 0;

			other->dirY = 0;

			other->y = self->y + self->h / 2;

			other->x = self->x + (self->w - other->w) / 2;

			self->target = other;

			self->action = &moveToEnd;

			self->active = FALSE;

			self->dirY = -self->speed;

			self->dirX = 1;

			/* Allow grabbing to be resumed when reloading the game */

			self->health = -1;
		}
	}
}

static void moveToStart()
{
	self->targetX = self->startX;
	self->targetY = self->startY;

	self->action = &moveToTarget;

	self->active = FALSE;
}

static void moveToEnd()
{
	self->targetX = self->endX;
	self->targetY = self->endY;

	self->action = &moveToTarget;

	self->active = TRUE;
}

static void activate(int val)
{
	self->active = TRUE;
}

static void addChain(Entity *parent)
{
	int chainHeight, i, chainCount, y;
	Entity *e, *previous;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Grabber Chain");
	}

	loadProperties("item/grabber_chain", e);

	chainHeight = e->h;

	chainCount = (int)parent->weight;

	e->inUse = FALSE;

	y = parent->y;

	previous = parent;

	for (i=0;i<chainCount;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Grabber Chain");
		}

		loadProperties("item/grabber_chain", e);

		e->x = parent->x + parent->w / 2;

		e->x -= e->w / 2;

		e->y = y - e->h;

		e->action = &chainWait;
		e->touch = NULL;

		e->head = previous;

		e->draw = &drawLoopingAnimationToMap;

		setEntityAnimation(e, STAND);

		y -= chainHeight;

		previous = e;
	}
}

static void chainWait()
{
	self->x = self->head->x + self->head->w / 2;
	self->x -= self->w / 2;

	self->y = self->head->y;
	self->y -= self->h;
}

static void init()
{
	addChain(self);

	self->action = &wait;
}
