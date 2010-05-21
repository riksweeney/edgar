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
#include "../item/key_items.h"
#include "../event/trigger.h"
#include "../hud.h"
#include "../inventory.h"
#include "../world/target.h"
#include "../player.h"
#include "../system/error.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void attackFinished(void);
static void mouthWait(void);
static void addMouth(void);
static void takeDamage(Entity *, int);
static void tentacleWait(void);
static void addTentacles(void);
static void tentacleAttackInit(void);
static void tentacleAttack(void);
static void tentacleAttackFinish(void);
static void wait(void);
static void tentacleAttackWait(void);
static int drawTentacle(void);
static void tentacleTakeDamage(Entity *, int);
static void tentacleAttackRetract(void);

Entity *addBorerBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Borer Boss");
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
	if (self->active == TRUE)
	{
		addMouth();
		
		addTentacles();
		
		self->mental = -1;
		
		self->targetY = -1;
		
		self->action = &doIntro;
		
		setContinuePoint(FALSE, self->name, NULL);
		
		playSoundToMap("sound/boss/ant_lion/earthquake.ogg", BOSS_CHANNEL, self->x, self->y, -1);
		
		shakeScreen(LIGHT, -1);
	}
}

static void addTentacles()
{
	int i, h;
	Entity *e;
	
	h = TILE_SIZE + TILE_SIZE / 2;
	
	for (i=0;i<3;i++)
	{
		e = getFreeEntity();
		
		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Borer Boss Tentacle Head");
		}

		loadProperties("boss/borer_boss_tentacle", e);
		
		setEntityAnimation(e, STAND);

		e->y = self->y + h - e->h / 2;
		
		e->mental = i + 1;
		
		e->startY = e->y - 3;
		e->endY = e->y + 3;
		
		e->face = RIGHT;
		
		e->touch = &entityTouch;
		
		e->takeDamage = &tentacleTakeDamage;
		
		e->die = &entityDieNoDrop;

		e->action = &tentacleWait;

		e->draw = &drawTentacle;

		e->type = ENEMY;
		
		e->head = self;
		
		h += TILE_SIZE * 2;
	}
	
	self->targetX = 3;
}

static void addMouth()
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Borer Boss Mouth");
	}

	loadProperties("boss/borer_boss_mouth", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &mouthWait;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;
	
	e->head = self;

	setEntityAnimation(e, STAND);
}

static void doIntro()
{
	self->x -=0.25;
	
	if (self->x <= self->startX)
	{
		self->x = self->startX;
		
		shakeScreen(LIGHT, 0);
		
		stopSound(BOSS_CHANNEL);
		
		self->action = &introPause;
	}
}

static void introPause()
{
	self->action = &attackFinished;

	playBossMusic();

	initBossHealthBar();
	
	self->takeDamage = &takeDamage;

	self->touch = &entityTouch;

	self->mental = 0;
	
	self->targetY = 0;

	self->endY = self->y;
}

static void attackFinished()
{
	self->thinkTime = 60;
	
	self->action = &wait;
}

static void wait()
{
	if (self->targetX > 0)
	{
		if (self->mental > 0)
		{
			if (prand() % 1 == 0)
			{
				self->targetY = 1 + prand() % self->targetX;
				
				if (self->targetY == self->mental)
				{
					self->targetY = -1;
				}
				
				printf("Sending out %d too in addition to %d\n", self->targetY, self->mental);
			}
			
			else
			{
				self->targetY = -1;
			}
			
			printf("Waiting for %d attack to finish\n", self->mental);
			
			self->action = &tentacleAttackWait;
		}
	}
	
	else
	{
		printf("Will do next set of attacks\n");
	}
}

static void tentacleAttackWait()
{
	if ((self->mental == -1 && self->targetY == -1) || self->targetX <= 0)
	{
		self->mental = 0;
		self->targetY = 0;
		
		self->action = &attackFinished;
	}
}

static void takeDamage(Entity *other, int damage)
{
	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		/* Minimum damage */
		
		self->health--;		

		if (other->type == PROJECTILE)
		{
			other->target = self;
		}

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
}

static void mouthWait()
{
	self->face = self->head->face;
	
	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}
	
	self->y = self->head->y + self->offsetY;
	
	if (self->head->flags & FLASH)
	{
		self->head->flags |= FLASH;
	}
	
	else
	{
		self->head->flags &= ~FLASH;
	}
}

static void tentacleWait()
{
	self->x = self->head->x - 12;
	
	self->endX = self->head->x + self->head->w / 2;
	
	if (self->head->mental <= -100)
	{
		self->mental = self->head->mental / -100;
		
		printf("Resetting number to %d\n", self->mental);
		
		if (self->mental == self->head->targetX)
		{
			printf("All done\n");
			
			self->head->mental = 0;
		}
		
		else
		{
			self->head->mental -= 100;
		}
	}
	
	else if ((self->head->mental == 0 && collision(self->x - SCREEN_WIDTH, self->y, SCREEN_WIDTH, self->h, player.x, player.y, player.w, player.h) == 1)
		|| self->head->targetY == self->mental)
	{
		if (self->head->mental == 0)
		{
			self->head->mental = self->mental;
		}
		
		self->thinkTime = self->mental == self->head->targetY ?  90 : 60;
		
		self->startX = self->x;
		
		self->dirY = 3.0f * (prand() % 2 == 0 ? -1 : 1);
		
		self->action = &tentacleAttackInit;
	}
}

static void tentacleAttackInit()
{
	self->y += self->dirY;
	
	if (self->y >= self->endY)
	{
		self->y = self->endY;
		
		self->dirY *= -1;
	}
	
	else if (self->y <= self->startY)
	{
		self->y = self->startY;
		
		self->dirY *= -1;
	}
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->y = self->startY + 8;
		
		self->dirX = -20;
		
		self->dirY = 0;
		
		self->action = &tentacleAttack;
	}
}

static void tentacleAttack()
{
	float dirX;
	
	dirX = self->dirX;
	
	checkToMap(self);
	
	if (self->dirX == 0)
	{
		if (dirX != 0)
		{
			shakeScreen(MEDIUM, 15);
		}
		
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->dirX = 3;
			
			self->thinkTime = 45;
			
			self->action = &tentacleAttackRetract;
		}
	}
	
	self->box.w = self->startX - self->x;
}

static void tentacleAttackRetract()
{
	self->thinkTime--;
	
	self->x += self->dirX;
	
	if (self->thinkTime <= 0)
	{
		self->thinkTime = 60;
		
		self->dirX = 16;
		
		self->action = tentacleAttackFinish;
	}
	
	self->box.w = self->startX - self->x;
}

static void tentacleAttackFinish()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->x += self->dirX;
		
		if (self->x >= self->startX)
		{
			self->action = &tentacleWait;
			
			printf("%d is back. %d %d\n", self->mental, self->head->mental, self->head->targetY);
			
			if (self->mental == self->head->mental)
			{
				printf("%d is back. Setting mental to -1\n", self->mental);
				
				self->head->mental = -1;
			}
			
			else if (self->mental == self->head->targetY)
			{
				printf("%d is back. Setting targetY to -1\n", self->mental);
				
				self->head->targetY = -1;
			}
			
			self->box.w = self->w;
		}
	}
	
	self->box.w = self->startX - self->x;
}

static int drawTentacle()
{
	int startX;

	startX = self->x;

	/* Draw the tentacle first */

	self->x += self->w;

	setEntityAnimation(self, WALK);

	while (self->x <= self->endX)
	{
		drawSpriteToMap();

		self->x += self->w;
	}

	/* Draw the tip */

	setEntityAnimation(self, STAND);

	self->x = startX;

	drawLoopingAnimationToMap();

	return TRUE;
}

static void tentacleTakeDamage(Entity *other, int damage)
{
	if (damage != 0)
	{
		self->health -= damage;

		if (other->type == PROJECTILE)
		{
			other->target = self;
		}

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
			if (self->head->mental == self->mental)
			{
				printf("%d died. Setting mental to -1\n", self->mental);
				
				self->head->mental = -1;
			}
			
			else if (self->head->targetY == self->mental)
			{
				printf("%d died. Setting targetY to -1\n", self->mental);
				
				self->head->targetY = -1;
			}
			
			self->dirX = 0;
			
			self->head->mental = -100;
			
			self->head->targetX--;
			
			self->damage = 0;

			self->die();
		}
	}
}
