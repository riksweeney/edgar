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
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../system/error.h"

extern Entity *self, player;

static void die(void);
static void dieFinish(void);
static void initialise(void);
static void wakeUp(void);
static void doIntro(void);
static void attackFinished(void);
static void takeDamage(Entity *, int);
static void regenerateHealth(void);
static void wait(void);
static void armourTakeDamage(Entity *, int);
static void armourDie(void);
static void armourWait(void);
static void regenerateArmour(void);

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

	setEntityAnimation(e, CUSTOM_1);

	return e;
}

static void initialise()
{
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

	else
	{
		/* Add Zs */


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

		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void wait()
{
	regenerateHealth();

	checkToMap(self);
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
			}

			else
			{
				self->startX++;
			}

			if (self->health <= 0)
			{
				self->damage = 0;

				self->startX = self->x;

				self->startY = 0;

				self->thinkTime = 180;

				self->action = &die;
			}

			else
			{
				enemyPain();
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

	prev = self;

	for (i=0;i<8;i++)
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

		e->mental = 60;

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

				e->action = e->die;

				self->mental--;
			}

			else
			{
				setCustomAction(e, &flashWhite, 6, 0, 0);

				setCustomAction(e, &invulnerableNoFlash, 20, 0, 0);
			}
		}

		break;
	}
}

static void armourDie()
{
	self->thinkTime--;

	if (self->thinkTime < 90)
	{
		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= NO_DRAW;
		}
	}

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}

	self->flags &= ~FLY;

	checkToMap(self);
}
