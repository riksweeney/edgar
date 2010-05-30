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
#include "../audio/audio.h"
#include "../enemy/enemies.h"
#include "../item/item.h"
#include "../graphics/decoration.h"
#include "../system/error.h"
#include "../system/random.h"

extern Entity *self, player;

static void wait(void);
static void followPlayer(void);
static void drop(void);
static void rise(void);
static void init(void);
static void armWait(void);
static int drawArm(void);
static void createArm(void);
static void touch(Entity *);

Entity *addLabCrusher(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Laboratory Crusher");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->startX = x;
	e->startY = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->die = &entityDieNoDrop;
	e->pain = NULL;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	createArm();
	
	self->action = &wait;
}

static void wait()
{
	if (self->active == TRUE)
	{
		self->action = &followPlayer;
	}
	
	checkToMap(self);
}

static void followPlayer()
{
	if (self->active == TRUE)
	{
		self->targetX = player.x - self->w / 2 + player.w / 2;

		/* Position above the player */

		if (abs(self->x - self->targetX) <= abs(self->dirX))
		{
			self->x = self->targetX;

			self->dirX = 0;
			
			self->action = &drop;
			
			self->thinkTime = 15;
		}

		else
		{
			self->dirX = self->targetX < self->x ? -player.speed * 3 : player.speed * 3;
		}
	}

	checkToMap(self);
	
	if (self->x < self->startX)
	{
		self->dirX = 0;
		
		self->x = self->startX;
	}
	
	else if (self->x > self->endX)
	{
		self->dirX = 0;
		
		self->x = self->endX;
	}
}

static void drop()
{
	int i;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->dirY = 16;
	}
	
	checkToMap(self);
	
	if (self->flags & ON_GROUND)
	{
		for (i=0;i<20;i++)
		{
			addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
		}
		
		playSoundToMap("sound/common/crunch.ogg", -1, self->x, self->y, 0);
		
		self->thinkTime = 120;
		
		self->action = &rise;
	}
}

static void rise()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->dirY = -8;
		
		if (self->y <= self->startY)
		{
			self->y = self->startY;
			
			self->mental--;
			
			if (self->mental <= 0)
			{
				self->active = FALSE;
			}
			
			self->dirY = 0;
			
			self->action = &followPlayer;
		}
	}
	
	checkToMap(self);
}

static void createArm()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Crusher Arm");
	}

	loadProperties("item/crusher_arm", e);

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &armWait;

	e->draw = &drawArm;

	e->head = self;

	setEntityAnimation(e, STAND);
}

static void armWait()
{
	checkToMap(self);

	self->x = self->head->x;
	self->y = self->head->y - self->h;
}

static int drawArm()
{
	int y;
	
	y = self->head->endY - self->h * 2;
	
	drawLoopingAnimationToMap();

	while (self->y >= y)
	{
		drawSpriteToMap();

		self->y -= self->h;
	}

	return TRUE;
}

static void touch(Entity *other)
{
	int bottomBefore;
	float dirX;

	if (other->dirY > 0)
	{
		/* Trying to move down */

		if (collision(other->x, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			bottomBefore = other->y + other->h - other->dirY - 1;

			if (abs(bottomBefore - self->y) < self->h - 1)
			{
				if (self->dirY < 0)
				{
					other->y -= self->dirY;

					other->dirY = self->dirY;

					dirX = other->dirX;

					other->dirX = 0;

					checkToMap(other);

					other->dirX = dirX;
				}

				/* Place the player as close to the solid tile as possible */

				other->y = self->y;
				other->y -= other->h;

				other->standingOn = self;
				other->dirY = 0;
				other->flags |= ON_GROUND;
			}
			
			else
			{
				entityTouch(other);
			}
		}
	}
	
	else
	{
		entityTouch(other);
	}
}
