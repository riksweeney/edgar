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
#include "../graphics/decoration.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../custom_actions.h"
#include "../system/error.h"
#include "../game.h"
#include "../item/item.h"
#include "../hud.h"
#include "../map.h"
#include "../projectile.h"

extern Entity *self, player;

static void walk(void);
static void changeWalkDirectionStart(void);
static void changeWalkDirection(void);
static void changeWalkDirectionFinish(void);
static void entityWait(void);
static void iceAttackStart(void);
static void iceAttack(void);
static void createIce(void);
static void iceAttackFinish(void);
static void iceBallMove(void);
static void iceFloorWait(void);
static void iceSpikeMove(void);
static void spikeTakeDamage(Entity *, int);

Entity *addIceTortoise(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Ice Tortoise");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &walk;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &entityDie;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void walk()
{
	moveLeftToRight();
	
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirX = 0;

		if (prand() % 5 == 0)
		{
			self->action = &changeWalkDirectionStart;
		}

		else
		{
			self->thinkTime = 60;

			self->action = &iceAttackStart;
		}
	}
}

static void changeWalkDirectionStart()
{
	self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
	
	setEntityAnimation(self, CUSTOM_1);

	self->action = &entityWait;

	self->animationCallback = &changeWalkDirection;

	self->thinkTime = 60;
	
	checkToMap(self);
}

static void changeWalkDirection()
{
	self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
	
	self->thinkTime--;

	self->action = &changeWalkDirection;

	setEntityAnimation(self, CUSTOM_3);

	if (self->thinkTime <= 0)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->frameSpeed = -1;

		setEntityAnimation(self, CUSTOM_1);

		self->animationCallback = &changeWalkDirectionFinish;

		self->action = &entityWait;
	}
	
	checkToMap(self);
}

static void entityWait()
{
	self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
	
	checkToMap(self);
}

static void changeWalkDirectionFinish()
{
	self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
	
	self->frameSpeed = 1;

	setEntityAnimation(self, STAND);

	self->action = &walk;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	self->thinkTime = 300 + prand() % 180;
	
	checkToMap(self);
}

static void iceAttackStart()
{
	self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;

	self->frameSpeed = 0;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->frameSpeed = 1;

		setEntityAnimation(self, CUSTOM_1);

		self->animationCallback = &createIce;
	}

	checkToMap(self);
}

static void createIce()
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Ice Tortoise Ice Ball");
	}
	
	loadProperties("enemy/ice_tortoise_ice_ball", e);
	
	setEntityAnimation(e, STAND);

	e->x = self->x + 71;
	e->y = self->y + 21;
	
	e->dirX = 2;
	e->dirY = -2;

	e->action = &iceBallMove;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->fallout = &entityDieNoDrop;
	
	e = getFreeEntity();
	
	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Ice Tortoise Ice Ball");
	}
	
	loadProperties("enemy/ice_tortoise_ice_ball", e);
	
	setEntityAnimation(e, STAND);

	e->x = self->x + self->w - e->w - 71;
	e->y = self->y + 21;
	
	e->dirX = -2;
	e->dirY = -2;

	e->action = &iceBallMove;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->fallout = &entityDieNoDrop;
	
	self->frameSpeed = 1;

	setEntityAnimation(self, CUSTOM_3);

	self->action = &iceAttack;

	self->thinkTime = 120;
}

static void iceAttack()
{
	self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
	
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->frameSpeed = -1;

		setEntityAnimation(self, CUSTOM_1);

		self->animationCallback = &iceAttackFinish;

		self->action = &entityWait;
	}

	checkToMap(self);
}

static void iceAttackFinish()
{
	self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
	
	setEntityAnimation(self, STAND);

	self->frameSpeed = 1;

	self->action = &walk;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	self->thinkTime = 120 + prand() % 120;

	checkToMap(self);
}

static void iceBallMove()
{
	int x;
	
	checkToMap(self);
	
	if (self->flags & ON_GROUND)
	{
		x = mapTileAt(self->x / TILE_SIZE, (self->y + self->h + 5) / TILE_SIZE);
		
		if (x >= SOLID_TILE_START && x <= SOLID_TILE_END)
		{
			self->layer = MID_GROUND_LAYER;
			
			x = self->x + self->w / 2;
			
			setEntityAnimation(self, WALK);
			
			self->x = x - self->w / 2;
			
			self->action = &iceFloorWait;
			
			self->y++;
			
			self->thinkTime = 30;
		}
		
		else
		{
			self->inUse = FALSE;
		}
	}
	
	else if (self->standingOn != NULL)
	{
		self->inUse = FALSE;
	}
	
	else
	{
		self->health--;
		
		if (self->health <= 0)
		{
			self->inUse = FALSE;
		}
	}
}

static void iceFloorWait()
{
	Entity *e;
	
	self->thinkTime--;
	
	if (self->thinkTime == 0)
	{
		e = getFreeEntity();
		
		if (e == NULL)
		{
			showErrorAndExit("No free slots to add an Upside Down Ice Spike");
		}
		
		loadProperties("enemy/ice_spike_upside_down", e);
		
		setEntityAnimation(e, STAND);

		e->x = self->x;
		e->y = self->y + self->h + 8;
		
		e->startY = self->y + self->h - e->h;

		e->action = &iceSpikeMove;
		e->touch = &entityTouch;
		e->takeDamage = &spikeTakeDamage;
		e->draw = &drawLoopingAnimationToMap;
		
		e->head = self;
	}
}

static void iceSpikeMove()
{
	int i;
	Entity *e;
	
	self->dirX = 0;
	
	if (self->y > self->startY)
	{
		self->y -= 4;
		
		if (self->y <= self->startY)
		{
			self->y = self->startY;
			
			if (self->head != NULL)
			{
				self->head->inUse = FALSE;
			
				self->head = NULL;
			}
		}
	}
	
	else
	{
		checkToMap(self);
		
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->takeDamage = NULL;
			
			playSoundToMap("sound/common/shatter.ogg", -1, self->x, self->y, 0);

			for (i=0;i<8;i++)
			{
				e = addTemporaryItem("misc/ice_spike_upside_down_piece", self->x, self->y, RIGHT, 0, 0);

				e->x = self->x + self->w / 2;
				e->x -= e->w / 2;

				e->y = self->y + self->h / 2;
				e->y -= e->h / 2;

				e->dirX = (prand() % 4) * (prand() % 2 == 0 ? -1 : 1);
				e->dirY = ITEM_JUMP_HEIGHT * 2 + (prand() % ITEM_JUMP_HEIGHT);

				setEntityAnimation(e, i);

				e->thinkTime = 60 + (prand() % 60);

				e->touch = NULL;
			}

			self->inUse = FALSE;
			
			if (self->head != NULL)
			{
				self->head->inUse = FALSE;
			}
		}
	}
}

static void spikeTakeDamage(Entity *other, int damage)
{
	Entity *temp;
	
	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

			enemyPain();
		}

		else
		{
			self->thinkTime = 0;
			
			self->takeDamage = NULL;
		}
		
		addDamageScore(damage, self);
	}
}
