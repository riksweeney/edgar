/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "../graphics/graphics.h"
#include "../entity.h"
#include "../player.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../geometry.h"
#include "../custom_actions.h"
#include "../projectile.h"
#include "../audio/audio.h"
#include "../system/error.h"
#include "../collisions.h"

extern Entity *self, player, playerShield;

static void creditsMove(void);
static void lookForPlayer(void);
static void confuseAttack(void);
static void confuseAttackFinish(void);
static void init(void);
static void rayTouch(Entity *);
static void hover();

Entity *addGhost(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a G-G-G-G-G-Ghost!");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->takeDamage = NULL;
	e->touch = &entityTouch;
	e->reactToBlock = &changeDirection;
	e->die = &entityDie;
	
	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	self->maxThinkTime = self->alpha;
	
	self->action = &lookForPlayer;
}

static void lookForPlayer()
{
	moveLeftToRight();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}

	if (player.health > 0 && self->thinkTime <= 0)
	{
		/* Must be within a certain range */

		if (collision(self->x + (self->face == LEFT ? -180 : self->w), self->y, 180, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->targetX = 3;
			
			self->dirX = 0;

			self->thinkTime = 30;

			self->action = &confuseAttack;
			
			setEntityAnimation(self, "ATTACK");
		}
	}
	
	self->mental--;
	
	if (self->mental <= 0)
	{
		self->mental = 0;
		
		self->takeDamage = NULL;
		
		self->alpha = self->maxThinkTime;
	}
	
	hover();
}

static void confuseAttack()
{
	Entity *e;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		e = addProjectile("enemy/ghost_reverse_ray", self, self->x + (self->face == RIGHT ? self->w : 0), self->y + self->h / 2, (self->face == RIGHT ? 4 : -4), 0);
		
		if (self->face == LEFT)
		{
			e->x = self->x + self->w - e->w - self->offsetX;
		}

		else
		{
			e->x = self->x + self->offsetX;
		}

		e->y = self->y + self->offsetY;

		e->touch = &rayTouch;
		
		self->targetX--;
		
		if (self->targetX <= 0)
		{
			self->thinkTime = 120;
			
			self->action = &confuseAttackFinish;
		}
		
		else
		{
			self->thinkTime = 2;
		}
	}
	
	checkToMap(self);
	
	hover();
}

static void rayTouch(Entity *other)
{
	if (other->type == PLAYER && !(other->flags & INVULNERABLE) && other->health > 0)
	{
		if ((other->flags & BLOCKING) && ((self->dirX > 0 && player.face == LEFT) || (self->dirX < 0 && player.face == RIGHT)))
		{
			player.dirX = self->dirX < 0 ? -2 : 2;

			checkToMap(&player);

			setCustomAction(&player, &helpless, 2, 0, 0);

			if (playerShield.thinkTime <= 0)
			{
				playSoundToMap("sound/edgar/block.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

				playerShield.thinkTime = 5;
			}
			
			self->parent = other;

			self->dirX = -self->dirX;
			
			self->face = self->face == LEFT ? RIGHT : LEFT;
		}

		else
		{
			setPlayerConfused(600);
			
			self->inUse = FALSE;
		}
	}
	
	else if (self->parent->type == PLAYER && strcmpignorecase(other->name, "enemy/ghost") == 0)
	{
		other->mental = 300;
		
		other->alpha = 255;
		
		other->takeDamage = &entityTakeDamageNoFlinch;
		
		self->inUse = FALSE;
	}
}

static void confuseAttackFinish()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->dirX = self->face == LEFT ? -self->speed : self->speed;
		
		self->thinkTime = 180;
		
		self->action = &lookForPlayer;
		
		setEntityAnimation(self, "STAND");
	}
	
	checkToMap(self);
	
	hover();
}

static void hover()
{
	self->startX += 4;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 4;
}

static void creditsMove()
{
	self->face = RIGHT;
	
	setEntityAnimation(self, "STAND");
	
	self->dirX = self->speed;
	
	checkToMap(self);
	
	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
	
	hover();
}
