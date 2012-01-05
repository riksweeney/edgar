/*
Copyright (C) 2009-2012 Parallel Realities

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

#include "../audio/audio.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../inventory.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void rise(void);
static void attackPlayer(void);
static void touch(Entity *);
static void stealItem(void);
static void leave(void);
static void sink(void);
static Entity *getRandomItem(void);
static void creditsMove(void);

Entity *addZombie(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Zombie");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &rise;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->touch = &touch;
	e->takeDamage = &entityTakeDamageFlinch;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void rise()
{
	facePlayer();
	
	if (self->y > self->startY)
	{
		self->y--;
	}
	
	else
	{
		self->y = self->startY;
		
		self->action = &attackPlayer;
	}
}

static void attackPlayer()
{
	self->dirX = player.x < self->x ? -self->speed : self->speed;
	
	checkToMap(self);
}

static void touch(Entity *other)
{
	if (self->health <= 0)
	{
		return;
	}

	if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}
	}

	else if (other->type == PROJECTILE && other->parent != self)
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}

		other->inUse = FALSE;
	}

	else if (self->target == other && !(self->flags & GRABBING))
	{
		self->targetX = player.x;
		
		player.flags |= GROUNDED;

		self->action = &stealItem;
		
		self->thinkTime = 300;

		self->flags |= GRABBING;

		self->layer = FOREGROUND_LAYER;
	}
}

static void stealItem()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->target = getRandomItem();
		
		if (self->target != NULL)
		{
			setCustomAction(self->target, &invulnerableNoFlash, 15, 0, 0);
		}
		
		self->face = self->startX < self->x ? LEFT : RIGHT;
		
		self->thinkTime = 600;
		
		self->action = &leave;
	}
}

static void leave()
{
	self->dirX = self->face == LEFT ? -self->speed : self->speed;
	
	if (self->target != NULL)
	{
		self->target->x = self->x + self->w / 2 - self->target->w / 2;
		self->target->y = self->y + self->h / 2 - self->target->h / 2;

		setCustomAction(self->target, &invulnerableNoFlash, 15, 0, 0);
	}
	
	checkToMap(self);
	
	self->thinkTime--;
	
	if (fabs(self->startX - self->x) <= fabs(self->dirX) || self->thinkTime <= 0)
	{
		self->dirX = 0;
		
		self->action = &sink;
	}
}

static void sink()
{
	self->y++;
	
	if (self->y >= self->endY)
	{
		if (self->target != NULL)
		{
			self->target->inUse = FALSE;
		}
		
		self->inUse = FALSE;
	}
}

static Entity *getRandomItem()
{
	int size, i;
	Entity *e;
	char itemName[MAX_VALUE_LENGTH];
	char *items[] = {
		"item/health_potion",
		"weapon/lightning_sword",
		"weapon/lightning_sword_empty",
		"item/instruction_card",
		"weapon/normal_arrow",
		"weapon/flaming_arrow",
		"item/full_soul_bottle",
		"item/tortoise_shell",
		"item/summoner_staff",
		"item/flaming_arrow_potion",
		"item/resurrection_amulet",
		"item/bomb",
		"item/spike_ball"
	};

	size = sizeof(items) / sizeof(char *);

	i = prand() % size;

	e = getInventoryItemByName(items[i]);

	if (e != NULL)
	{
		if (strcmpignorecase(e->name, "item/health_potion") == 0)
		{
			e->mental = -1;
		}
		
		STRNCPY(itemName, e->objectiveName, sizeof(itemName));

		removeInventoryItemByName(e->name);
	}
	
	return e;
}

static void creditsMove()
{
	self->face = RIGHT;

	setEntityAnimation(self, "WALK");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}
