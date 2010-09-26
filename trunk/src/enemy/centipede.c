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
static void touch(Entity *);
static void headMove(void);
static void addSegments(void);
static void segmentMove(void);
static void segmentInit(void);
static void reactToBlock(void);
static void segmentTakeDamage(Entity *, int);
static void becomeHead(void);
static void greenTouch(Entity *);
static void becomeGreen(void);

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
	e->touch = &touch;
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
	
	self->action = &headMove;
}

static void headMove()
{
	if (self->mental >= 2)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			loadProperties(self->mental == 2 ? "enemy/yellow_centipede_segment" : "enemy/red_centipede_segment", self);
			
			self->mental = self->mental == 2 ? 0 : 1;
			
			self->touch = &touch;
		}
	}
	
	moveLeftToRight();
}

static void addSegments()
{
	char name[MAX_VALUE_LENGTH];
	int i, frameCount, mental;
	Entity *e, *prev;
	
	prev = self;
	
	mental = strcmpignorecase(self->name, "enemy/red_centipede") == 0 ? 1 : 0;
	
	snprintf(name, MAX_VALUE_LENGTH, "%s_segment", self->name);
	
	for (i=0;i<self->mental;i++)
	{
		e = getFreeEntity();
		
		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Centipede Segment");
		}

		loadProperties(name, e);

		e->action = &segmentInit;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;
		e->takeDamage = &segmentTakeDamage;
		e->die = &entityDieNoDrop;

		e->type = ENEMY;
		
		e->face = self->face;
		
		e->target = prev;
		
		e->head = self;
		
		e->speed = self->speed;

		setEntityAnimation(e, STAND);
		
		frameCount = getFrameCount(e);
		
		e->currentFrame = prand() % frameCount;
		
		e->x = self->x;
		e->y = self->y;
		
		e->mental = mental;
		
		prev = e;
		
		if (i == 0)
		{
			self->head = e;
		}
	}
	
	self->mental = mental;
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
	if (self->target->mental >= 2)
	{
		if (self->mental < 2)
		{
			loadProperties("enemy/green_centipede_segment", self);
			
			self->mental = self->target->mental;
			
			self->touch = &greenTouch;
		}
		
		else
		{
			self->thinkTime--;
			
			if (self->thinkTime <= 0)
			{
				loadProperties(self->mental == 2 ? "enemy/yellow_centipede_segment" : "enemy/red_centipede_segment", self);
				
				self->mental = self->mental == 2 ? 0 : 1;
				
				self->touch = &entityTouch;
			}
		}
	}
	
	if (self->mental == 0 && self->head->health <= 0)
	{
		self->health = 0;
		
		entityDie();
	}
	
	if (self->target->health <= 0)
	{
		if (self->mental == 1)
		{
			self->action = &becomeHead;
		}
		
		else
		{
			self->health = 0;
			
			entityDieNoDrop();
		}
	}
	
	/* If segment is facing the same way as the target then just move with it */
	
	if (self->face == LEFT && self->x + self->w <= self->target->x)
	{
		self->x = self->target->x - self->w;
		
		self->face = self->target->face;
	}
	
	else if (self->face == RIGHT && self->x >= self->target->x + self->target->w)
	{
		self->x = self->target->x + self->target->w;
		
		self->face = self->target->face;
	}
	
	else
	{
		self->dirX = self->face == LEFT ? -self->target->speed : self->target->speed;
	}
	
	checkToMap(self);
}

static void becomeHead()
{
	int x = self->x;
	
	loadProperties(self->head->name, self);

	self->action = &moveLeftToRight;
	self->die = &entityDie;
	self->touch = &touch;
	self->takeDamage = &entityTakeDamageNoFlinch;
	self->reactToBlock = &reactToBlock;
	self->pain = &enemyPain;
	
	self->flags &= ~UNBLOCKABLE;
	
	self->x = x;
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

			enemyPain();
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

static void touch(Entity *other)
{
	if (self->mental < 2 && strcmpignorecase(other->name, "item/spore") == 0)
	{
		self->action = &becomeGreen;
	}
	
	else
	{
		entityTouch(other);
	}
}

static void becomeGreen()
{
	loadProperties("enemy/green_centipede", self);
	
	self->mental = self->mental == 0 ? 2 : 3;
	
	self->touch = &greenTouch;
}

static void greenTouch(Entity *other)
{
	float y;
	
	if (other->type == PLAYER && other->dirY > 0)
	{
		y = other->y - other->dirY + other->box.h;
		
		if (y < self->y && other->y + other->h >= self->y)
		{
			other->y = self->y - other->h;
			
			other->standingOn = self;
			other->dirY = 0;
			other->flags |= ON_GROUND;
		}
	}
}
