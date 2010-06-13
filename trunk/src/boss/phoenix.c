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
#include "../enemy/rock.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "../projectile.h"
#include "../map.h"
#include "../game.h"
#include "../audio/music.h"
#include "../audio/audio.h"
#include "../graphics/gib.h"
#include "../graphics/decoration.h"
#include "../item/key_items.h"
#include "../event/trigger.h"
#include "../hud.h"
#include "../inventory.h"
#include "../world/target.h"
#include "../player.h"
#include "../system/error.h"
#include "../enemy/enemies.h"
#include "../item/item.h"
#include "../item/bomb.h"
#include "../geometry.h"
#include "../world/explosion.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void attackFinished(void);
static void flameAttackInit(void);
static void flameAttack(void);
static void dropAttackInit(void);
static void dropAttack(void);
static void riseAttackInit(void);
static void riseAttack(void);

Entity *addPhoenix(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Phoenix");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;
	e->touch = NULL;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	Target *t;
	
	if (self->active == TRUE)
	{
		if (cameraAtMinimum())
		{
			centerMapOnEntity(NULL);
			
			t = getTargetByName("PHOENIX_TARGET_RIGHT");
			
			if (t == NULL)
			{
				showErrorAndExit("Phoenix cannot find target");
			}
			
			self->dirY = -6;
			
			self->flags |= ATTACKING;
			
			self->targetY = t->y;
			
			setContinuePoint(FALSE, self->name, NULL);
			
			self->action = &doIntro;
			
			self->touch = &entityTouch;
		}
	}
}

static void doIntro()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		checkToMap(self);
		
		if (self->y <= self->targetY)
		{
			self->y = self->targetY;
			
			self->dirY = 0;
			
			self->thinkTime = 120;
			
			self->action = &introPause;
		}
	}
}

static void introPause()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		playDefaultBossMusic();

		initBossHealthBar();
		
		self->action = &attackFinished;
	}
}

static void entityWait()
{
	Target *t;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		if (prand() % 1000 == 0)
		{
			setEntityAnimation(self, ATTACK_1);
			
			self->thinkTime = 60;
			
			self->startX = prand() % 2;
			
			t = getTargetByName(self->startX == 0 ? "PHOENIX_TARGET_RIGHT" : "PHOENIX_TARGET_LEFT");
			
			if (t == NULL)
			{
				showErrorAndExit("Phoenix cannot find target");
			}
			
			self->targetX = t->x;
			self->targetY = t->y;
			
			self->face = self->startX == 0 ? LEFT : RIGHT;
			
			self->action = &flameAttackInit;
			
			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);
			
			self->dirX *= 2;
			self->dirY *= 2;
		}
		
		else
		{
			t = getTargetByName("PHOENIX_TARGET_LEFT");
			
			if (t == NULL)
			{
				showErrorAndExit("Phoenix cannot find target");
			}
			
			self->targetY = t->y;
			
			setEntityAnimation(self, STAND);
			
			self->maxThinkTime = 5;
			
			self->action = &dropAttackInit;
		}
	}
	
	checkToMap(self);	
}

static void attackFinished()
{
	self->thinkTime = 60;
	
	self->startX = 0;
	
	self->action = &entityWait;
}

static void flameAttackInit()
{
	if (self->dirX == 0 && self->dirY == 0)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->action = &flameAttack;
			
			self->dirX = self->face == LEFT ? -4 : 4;
		}
	}
	
	else if (atTarget())
	{
		self->dirX = 0;
		self->dirY = 0;
	}
	
	checkToMap(self);
}

static void flameAttack()
{
	Entity *e;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		e = addProjectile("boss/phoenix_fireball", self, self->x, self->y, 0, 8);
		
		playSoundToMap("sound/enemy/fireball/fireball.ogg", BOSS_CHANNEL, self->x, self->y, 0);
		
		e->x = self->x + self->w / 2 - e->w / 2;
		
		self->thinkTime = 3;
	}
	
	checkToMap(self);
	
	if (self->dirX == 0)
	{
		self->action = &attackFinished;
	}
}

static void dropAttackInit()
{
	self->targetX = player.x - self->w / 2 + player.w / 2;

	/* Position above the player */

	if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;
		
		self->action = &dropAttack;
		
		self->thinkTime = 60;
	}

	else
	{
		self->dirX = self->targetX < self->x ? -player.speed * 3 : player.speed * 3;
	}
	
	checkToMap(self);
}

static void dropAttack()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->dirY = 12;
	}
	
	checkToMap(self);
	
	if (self->y > self->startY)
	{
		self->y = self->startY;
		
		self->dirY = 0;
		
		self->action = &riseAttackInit;
	}
}

static void riseAttackInit()
{
	self->targetX = player.x - self->w / 2 + player.w / 2;

	/* Position below the player */

	if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;
		
		self->action = &riseAttack;
		
		self->thinkTime = 60;
	}

	else
	{
		self->dirX = self->targetX < self->x ? -player.speed * 3 : player.speed * 3;
	}
	
	checkToMap(self);
}

static void riseAttack()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->dirY = -12;
	}
	
	checkToMap(self);
	
	if (self->y < self->targetY)
	{
		self->y = self->targetY;
		
		self->dirY = 0;
		
		self->maxThinkTime--;
		
		self->action = self->maxThinkTime > 0 ? &dropAttackInit : &attackFinished;
	}
}
