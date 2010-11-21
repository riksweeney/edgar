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
#include "../system/random.h"
#include "../audio/audio.h"
#include "../audio/music.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../graphics/decoration.h"
#include "../game.h"
#include "../hud.h"
#include "../map.h"
#include "../item/key_items.h"
#include "../player.h"
#include "../enemy/enemies.h"
#include "../graphics/gib.h"
#include "../system/error.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void die(void);
static void attackFinished(void);
static void takeDamage(Entity *, int);
static void touch(Entity *);
static void introPause(void);
static void freezeBody(void);
static void immolateBody(void);
static void throwWallWalkerInit(void);
static void throwWallWalker(void);
static void chargePlayerInit(void);

Entity *addCaveBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Cave Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	
	e->touch = &entityTouch;

	e->die = &die;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	if (self->active == TRUE)
	{
		self->flags &= ~NO_DRAW;

		if (cameraAtMinimum())
		{
			centerMapOnEntity(NULL);

			self->action = &doIntro;

			self->thinkTime = 60;

			self->startX = 20;

			self->endX = 3;

			self->startY = 0;
			
			self->takeDamage = &takeDamage;
			
			self->touch = &touch;

			setContinuePoint(FALSE, self->name, NULL);
		}
	}

	checkToMap(self);
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		checkToMap(self);

		self->action = &introPause;

		setEntityAnimation(self, STAND);
	}
}

static void introPause()
{
	playDefaultBossMusic();

	initBossHealthBar();

	self->action = &attackFinished;

	checkToMap(self);
	
	self->startX = 0;
	
	self->endY = 25;
}

static void entityWait()
{
	int action;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		switch ((int)self->startX)
		{
			case 0:
				action = prand() % 4;
				
				switch (action)
				{
					case 0:
						self->action = &freezeBody;
					break;
					
					case 1:
						self->action = &immolateBody;
					break;
					
					case 2:
						self->action = &throwWallWalkerInit;
					break;
					
					default:
						self->action = &chargePlayerInit;
					break;
				}
			break;
			
			case 1: /* Fire */
				action = prand() % 3;
				
				switch (action)
				{
					case 0:
					case 1:
					case 2:
						self->action = &throwWallWalkerInit;
					break;
					
					default:
						self->action = &chargePlayerInit;
					break;
				}
			break;
			
			default: /* Ice */
				action = prand() % 3;
				
				switch (action)
				{
					case 0:
					case 1:
					case 2:
						self->action = &throwWallWalkerInit;
					break;
					
					default:
						self->action = &chargePlayerInit;
					break;
				}
			break;
		}
	}
	
	checkToMap(self);
}

static void throwWallWalkerInit()
{
	self->mental = 1 + prand() % 5;
	
	self->thinkTime = 30;
	
	self->action = &throwWallWalker;
	
	checkToMap(self);
}

static void throwWallWalker()
{
	Entity *e;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		e = addEnemy("enemy/wall_walker", self->x, self->y);
		
		e->face = self->face;
		
		e->dirY = ITEM_JUMP_HEIGHT;
		
		e->dirX = e->face == LEFT ? -e->speed : e->speed;
		
		self->mental--;
		
		self->thinkTime = 60;
		
		if (self->mental <= 0)
		{
			self->action = &attackFinished;
		}
	}
	
	checkToMap(self);
}

static void chargePlayerInit()
{
	
}

static void freezeBody()
{
	self->startX = 1;
	
	playSoundToMap("sound/common/freeze.ogg", BOSS_CHANNEL, self->x, self->y, 0);
	
	self->action = &attackFinished;
}

static void immolateBody()
{
	self->startX = 2;
	
	self->action = &attackFinished;
}

static void attackFinished()
{
	self->mental = 0;
	
	self->thinkTime = 120;

	self->action = &entityWait;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (self->startX == 0)
	{
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

				if (self->pain != NULL)
				{
					self->pain();
				}
			}

			else
			{
				self->takeDamage = NULL;
				
				self->thinkTime = 120;
				
				self->startX = self->x;
				
				self->damage = 0;
				
				self->endX = 0;
				
				self->action = &die;
			}
			
			addDamageScore(damage, self);
		}
	}
		
	else
	{		
		playSoundToMap("sound/common/dink.ogg", EDGAR_CHANNEL, self->x, self->y, 0);
		
		if (other->reactToBlock != NULL)
		{
			temp = self;

			self = other;

			self->reactToBlock();

			self = temp;
		}

		if (prand() % 10 == 0)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
		}

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
		
		damage = 0;
		
		addDamageScore(damage, self);
	}
}

static void die()
{
	Entity *e;

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		clearContinuePoint();

		increaseKillCount();

		freeBossHealthBar();

		e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

		e->dirY = ITEM_JUMP_HEIGHT;

		entityDieNoDrop();
	}
}

static void touch(Entity *other)
{
	if (other->type == KEY_ITEM && strcmpignorecase(other->name, "item_stalactite") == 0)
	{
		if (self->startX == -1)
		{
			self->takeDamage(other, 500);
			
			other->mental = -1;
		}
		
		else
		{
			other->action = &die;
		}
	}
	
	else if (self->startX == 1 && other->type == KEY_ITEM && strcmpignorecase(other->name, "item/ice_cube") == 0)
	{
		self->startY--;
		
		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		enemyPain();
		
		other->inUse = FALSE;
		
		if (self->startY <= 0)
		{
			self->startX = 0;
		}
	}
	
	else if (self->startX == 2 && other->type == PROJECTILE && strcmpignorecase(other->name, "weapon/flaming_arrow") == 0)
	{
		self->startY--;
		
		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		enemyPain();
		
		other->inUse = FALSE;
		
		if (self->startY <= 0)
		{
			self->startX = 0;
		}
	}
	
	else
	{
		entityTouch(other);
	}
}
