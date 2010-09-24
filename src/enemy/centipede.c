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
#include "../collisions.h"
#include "../custom_actions.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../system/error.h"

extern Entity *self;

static void init(void);
static void addSegments(void);
static void segmentMove(void);
static void segmentInit(void);
static void reactToBlock(void);
static void segmentTakeDamage(Entity *, int);

Entity *addCentipede(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Centipede");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &reactToBlock;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	addSegments();
	
	self->dirX = self->face == LEFT ? -self->speed : self->speed;
	
	self->action = &moveLeftToRight;
}

static void addSegments()
{
	int i;
	Entity *e, *prev;
	
	prev = self;
	
	for (i=0;i<self->mental;i++)
	{
		e = getFreeEntity();
		
		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Centipede Segment");
		}

		loadProperties("enemy/centipede_segment", e);

		e->action = &segmentInit;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;
		e->takeDamage = &segmentTakeDamage;
		e->die = &entityDieNoDrop;

		e->type = ENEMY;
		
		e->face = self->face;
		
		e->target = prev;

		setEntityAnimation(e, STAND);
		
		e->x = self->x;
		e->y = self->y;
		
		prev = e;
	}
}

static void segmentInit()
{
	if (self->face == LEFT && self->target->x + self->target->w <= self->x)
	{
		self->action = &segmentMove;
	}
	
	else if (self->face == RIGHT && self->target->x - self->target->w >= self->x)
	{
		self->action = &segmentMove;
	}
	
	checkToMap(self);
}

static void segmentMove()
{
	if (self->target->health <= 0)
	{
		self->health = 0;
		
		entityDieNoDrop();
	}
	
	/* If segment is facing the same way as the target then just move with it */
	
	if (self->face == self->target->face)
	{
		self->x = self->face == LEFT ? self->target->x + self->w : self->target->x - self->w;
		
		self->dirX = 0;
	}
	
	else if (self->face == LEFT && self->x <= self->target->x - self->target->w)
	{
		self->face = self->target->face;
	}
	
	else if (self->face == RIGHT && self->x >= self->target->x + self->target->w)
	{
		self->face = self->target->face;
	}
	
	else
	{
		self->dirX = self->face == LEFT ? -self->target->speed : self->target->speed;
	}
	
	checkToMap(self);
}

static void reactToBlock()
{
	self->endX = self->x;
	
	changeDirection();
}

static void segmentTakeDamage(Entity *other, int damage)
{
	Entity *temp;
	
	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);
			}

			if (self->pain != NULL)
			{
				self->pain();
			}
		}

		else
		{
			self->damage = 0;

			self->die();
		}
		
		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}
	}
}
