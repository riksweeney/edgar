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
#include "../system/random.h"
#include "../audio/audio.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../game.h"
#include "../audio/music.h"
#include "../collisions.h"
#include "../item/key_items.h"
#include "../item/item.h"
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../system/error.h"
#include "../event/script.h"

extern Entity *self, player;

static void die(void);
static void dieFinish(void);
static void initialise(void);
static void wakeUp(void);
static void doIntro(void);
static void introWait(void);
static void attackFinished(void);
static void takeDamage(Entity *, int);
static void regenerateHealth(void);
static void wait(void);
static void armourTakeDamage(Entity *, int);
static void armourDie(void);
static void armourWait(void);
static void regenerateArmour(void);
static void lookForPlayer(void);
static void addYellowGem(void);
static void gemWait(void);
static void zMove(void);
static void zVanish(void);

Entity *addArmourBoss(int x, int y, char *name)
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Armour Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;

	e->takeDamage = NULL;

	e->type = ENEMY;

	e->active = FALSE;

	e->die = &die;
	
	e->resumeNormalFunction = &attackFinished;

	setEntityAnimation(e, CUSTOM_1);

	return e;
}

static void initialise()
{
	Entity *e;
	
	if (self->active == TRUE)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, CUSTOM_2);

			self->animationCallback = &wakeUp;

			self->touch = &entityTouch;

			self->takeDamage = &takeDamage;
		}
	}

	else if (prand() % 120 == 0)
	{
		e = addBasicDecoration(self->x + self->w - 30, self->y + 30, "decoration/z");
		
		if (e != NULL)
		{
			e->face = RIGHT;
			
			e->startX = e->x;
			
			e->action = &zMove;
			e->animationCallback = &zVanish;
		}
	}
	
	if (self->endY == 0)
	{
		addYellowGem();
	}

	checkToMap(self);
}

static void wakeUp()
{
	setEntityAnimation(self, CUSTOM_3);

	self->thinkTime = 60;

	self->action = &doIntro;

	checkToMap(self);
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		initBossHealthBar();

		playBossMusic();

		self->action = &introWait;
		
		self->startX = 360;
		
		self->endX = 0;
		
		self->endY = 1;
		
		self->mental = 0;
		
		runScript("armour_boss_start");
	}

	checkToMap(self);
}

static void introWait()
{
	checkToMap(self);
}

static void wait()
{
	regenerateHealth();

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->thinkTime = 180 + (prand() % 240);

			self->action = &lookForPlayer;
		}
	}
}

static void lookForPlayer()
{
	setEntityAnimation(self, WALK);

	self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirX = 0;

		setEntityAnimation(self, STAND);

		self->thinkTime = 30;

		self->action = &wait;
	}

	regenerateHealth();
}

static void attackFinished()
{
	self->thinkTime = 30;

	self->action = &wait;

	regenerateHealth();

	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		if (self->mental > 0)
		{
			/* The armour will take the damage instead */

			armourTakeDamage(other, damage);
		}

		else
		{
			self->health -= damage;

			self->endX--;

			if (self->endX <= 0)
			{
				self->endX = 0;
			}

			if (other->type != ENEMY && self->mental == 0 && (prand() % 10 == 0))
			{
				setInfoBoxMessage(60, _("Its wounds are already healing..."));

				/* Don't get killed by anything except the slimes */

				if (self->health <= 0)
				{
					self->health = 1;
				}
			}

			if (strcmpignorecase(other->name, "enemy/red_baby_slime") != 0)
			{
				setCustomAction(self, &flashWhite, 6, 0, 0);

				setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);

				enemyPain();
			}

			else
			{
				self->startX++;

				if (self->endX == 0)
				{
					self->endX = 20;

					setCustomAction(self, &flashWhite, 6, 0, 0);

					enemyPain();
				}
			}

			if (self->health <= 0)
			{
				self->damage = 0;

				self->startX = self->x;

				self->startY = 0;

				self->thinkTime = 180;

				self->action = &die;
			}
		}
	}
}

static void regenerateHealth()
{
	self->health += 3;

	if (self->health > self->maxHealth)
	{
		self->health = self->maxHealth;
	}

	if (self->mental == 0)
	{
		self->startX--;

		if (self->startX <= 0)
		{
			regenerateArmour();
		}
	}
}

static void die()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 60;

		self->action = &dieFinish;

		for (i=0;i<60;i++)
		{
			e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

			if (e != NULL)
			{
				e->y -= prand() % e->h;
			}
		}

		setEntityAnimation(self, DIE);
	}

	else
	{
		self->x = self->startX + sin(DEG_TO_RAD(self->startY)) * 4;

		self->startY += 90;

		if (self->startY >= 360)
		{
			self->startY = 0;
		}
	}
}

static void dieFinish()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		increaseKillCount();

		freeBossHealthBar();

		e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

		e->dirY = ITEM_JUMP_HEIGHT;

		fadeBossMusic();

		entityDieNoDrop();
	}
}

static void regenerateArmour()
{
	int i;
	Entity *e, *prev;

	if (self->target == NULL)
	{
		prev = self;
		
		for (i=0;i<7;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add the Armour Boss's Armour");
			}

			loadProperties("boss/armour_boss_armour", e);

			e->x = self->x;
			e->y = self->y;

			e->action = &armourWait;

			e->draw = &drawLoopingAnimationToMap;

			e->takeDamage = NULL;

			e->type = ENEMY;

			e->die = &armourDie;

			e->thinkTime = 60 + prand() % 120;

			e->head = self;

			prev->target = e;

			prev = e;

			setEntityAnimation(e, i);

			if (self->face == LEFT)
			{
				e->x = self->x + self->w - e->w - e->offsetX;
			}

			else
			{
				e->x = self->x + e->offsetX;
			}

			e->y = self->y + e->offsetY;

			self->mental++;
		}
	}
	
	else
	{
		i = 0;
		
		for (e=self->target;e!=NULL;e=e->target)
		{
			loadProperties("boss/armour_boss_armour", e);
			
			e->x = self->x;
			e->y = self->y;

			e->action = &armourWait;

			e->draw = &drawLoopingAnimationToMap;

			e->takeDamage = NULL;

			e->type = ENEMY;

			e->die = &armourDie;

			e->thinkTime = 60 + prand() % 120;

			setEntityAnimation(e, i);

			if (self->face == LEFT)
			{
				e->x = self->x + self->w - e->w - e->offsetX;
			}

			else
			{
				e->x = self->x + e->offsetX;
			}

			e->y = self->y + e->offsetY;

			self->mental++;
			
			i++;
		}
	}
}

static void armourWait()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		self->flags |= INVULNERABLE;

		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= NO_DRAW;
		}
	}

	else
	{
		self->flags &= ~NO_DRAW;
	}

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
}

static void armourTakeDamage(Entity *other, int damage)
{
	Entity *e;

	for (e=self->target;e!=NULL;e=e->target)
	{
		if (e->health <= 0)
		{
			continue;
		}

		if (!(e->flags & INVULNERABLE))
		{
			e->health -= damage;

			if (e->health <= 0)
			{
				e->thinkTime = 120;
				
				e->flags &= ~FLY;
				
				e->dirY = ITEM_JUMP_HEIGHT;

				e->action = e->die;

				self->mental--;

				if (self->mental == 0)
				{
					self->startX = 1200;
				}
			}

			else
			{
				setCustomAction(e, &flashWhite, 6, 0, 0);

				setCustomAction(e, &invulnerableNoFlash, 20, 0, 0);

				enemyPain();
			}
		}

		break;
	}
}

static void armourDie()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;
		
		if (self->thinkTime < 90)
		{
			if (self->thinkTime % 3 == 0)
			{
				self->flags ^= NO_DRAW;
			}
		}
	}

	else
	{
		self->flags |= NO_DRAW;
	}

	checkToMap(self);
}

static void addYellowGem()
{
	Entity *e;
	
	e = addPermanentItem("item/yellow_gem", 0, 0);
	
	e->action = &gemWait;
	
	e->head = self;
	
	e->touch = NULL;
	
	self->endY = 1;
}

static void gemWait()
{
	self->face = self->head->face;
	
	setEntityAnimation(self, getAnimationTypeAtIndex(self->head));
	
	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;
}

static void zMove()
{
	self->health++;
	
	self->x = self->startX + sin(DEG_TO_RAD(self->health)) * 8;
	
	self->y -= 0.5;
}

static void zVanish()
{
	self->inUse = FALSE;
}
