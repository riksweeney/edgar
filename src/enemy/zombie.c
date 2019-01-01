/*
Copyright (C) 2009-2019 Parallel Realities

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
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include "../headers.h"

#include "../audio/audio.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../enemy/rock.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../inventory.h"
#include "../map.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player, playerWeapon;

static void rise(void);
static void attackPlayer(void);
static void touch(Entity *);
static void stealItem(void);
static void leave(void);
static void sink(void);
static void die(void);
static Entity *getRandomItem(void);
static void creditsMove(void);
static void creditsHeadMove(void);
static void fallOff(void);
static void headWait(void);
static void init(void);
static void walkOffScreen(void);

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

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->takeDamage = &entityTakeDamageFlinch;

	e->creditsAction = &init;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Zombie Head");
	}

	loadProperties("enemy/zombie_head", e);

	setEntityAnimation(e, self->animationName);

	if (e->face == LEFT)
	{
		e->x = self->x + self->w - e->w - e->offsetX;
	}

	else
	{
		e->x = self->x + e->offsetX;
	}

	e->y = self->y + e->offsetY;

	e->head = self;

	e->action = &headWait;
	e->draw = &drawLoopingAnimationToMap;

	e->creditsAction = &creditsHeadMove;

	self->action = &rise;

	self->creditsAction = &creditsMove;
}

static void rise()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		facePlayer();

		if (self->y > self->startY)
		{
			self->y -= 4;
		}

		else
		{
			playSoundToMap("sound/common/crumble", BOSS_CHANNEL, self->x, self->y, 0);

			e = addSmallRock(self->x, self->y, "common/small_rock");

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			e->dirX = -3;
			e->dirY = -8;

			e = addSmallRock(self->x, self->y, "common/small_rock");

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			e->dirX = 3;
			e->dirY = -8;

			self->touch = &touch;

			self->y = self->startY;

			self->thinkTime = 15;

			self->action = &attackPlayer;

			setEntityAnimation(self, "WALK");

			self->layer = MID_GROUND_LAYER;
		}
	}
}

static void attackPlayer()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		facePlayer();

		self->dirX = player.x < self->x ? -self->speed : self->speed;

		checkToMap(self);
	}
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

	else if (other->type == PLAYER && !(self->flags & GRABBING))
	{
		setEntityAnimation(self, "ATTACK_1");

		self->takeDamage = entityTakeDamageNoFlinch;

		self->dirX = 0;

		self->targetX = player.x;

		self->endX = self->x;

		self->action = &stealItem;

		self->thinkTime = 180;

		self->flags |= GRABBING;

		setCustomAction(&player, &stickToFloor, 3, 0, 0);
	}
}

static void stealItem()
{
	self->x = self->endX;

	self->layer = FOREGROUND_LAYER;

	setCustomAction(&player, &stickToFloor, 3, 0, 0);

	player.x = self->targetX;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "WALK");

		self->target = getRandomItem();

		if (self->target != NULL)
		{
			self->target->flags |= NO_DRAW;

			setInfoBoxMessage(180, 255, 255, 255, _("Your %s has been stolen!"), self->target->objectiveName);

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

		setEntityAnimation(self, "STAND");

		self->layer = BACKGROUND_LAYER;
	}
}

static void sink()
{
	self->y++;

	if (self->target != NULL)
	{
		setCustomAction(self->target, &invulnerableNoFlash, 15, 0, 0);
	}

	if (self->y >= self->endY)
	{
		if (self->target != NULL)
		{
			self->target->inUse = FALSE;
		}

		self->inUse = FALSE;
	}
}

static void die()
{
	if (self->target != NULL)
	{
		self->target->flags &= ~NO_DRAW;

		setCustomAction(self->target, &invulnerable, 60, 0, 0);

		self->target->dirY = ITEM_JUMP_HEIGHT;
	}

	if (self->dirX != 0)
	{
		setCustomAction(self, &invulnerableNoFlash, 600, 0, 0);

		self->action = &walkOffScreen;
	}

	else
	{
		entityDieNoDrop();
	}
}

static void walkOffScreen()
{
	int startX = getMapStartX();

	checkToMap(self);

	if (self->x + self->w < startX || self->x > startX + SCREEN_WIDTH)
	{
		self->inUse = FALSE;
	}
}

static Entity *getRandomItem()
{
	int size, i, count;
	Entity *e;
	char *items[] = {
		"item/health_potion",
		"weapon/normal_arrow",
		"weapon/flaming_arrow",
		"item/spike_ball",
		"weapon/lightning_sword",
		"weapon/lightning_sword_empty",
		"item/full_soul_bottle",
		"item/tortoise_shell",
		"item/summoner_staff",
		"item/keepsake",
		"item/flaming_arrow_potion",
		"item/resurrection_amulet",
		"item/bomb"
	};

	size = sizeof(items) / sizeof(char *);

	e = NULL;

	for (count=0;count<size;count++)
	{
		i = prand() % size;

		e = getInventoryItemByName(items[i]);

		if (e != NULL)
		{
			if (strcmpignorecase(e->name, "item/health_potion") == 0)
			{
				e->mental = -1;
			}

			if (strcmpignorecase(e->name, playerWeapon.name) == 0)
			{
				setPlayerLocked(TRUE);

				setPlayerLocked(FALSE);
			}

			e = addEntity(*e, self->x, self->y);

			removeInventoryItemByName(items[i]);

			break;
		}
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

static void headWait()
{
	self->layer = self->head->layer;

	self->face = self->head->face;

	setEntityAnimation(self, self->head->animationName);

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;

	if (self->head->health <= 0)
	{
		self->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		self->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		self->action = &fallOff;

		self->thinkTime = 180;
	}

	else
	{
		if (self->head->flags & NO_DRAW)
		{
			self->flags |= NO_DRAW;
		}

		else
		{
			self->flags &= ~NO_DRAW;
		}

		if (self->head->flags & FLASH)
		{
			self->flags |= FLASH;
		}

		else
		{
			self->flags &= ~FLASH;
		}
	}

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static void fallOff()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		self->thinkTime--;

		if (self->thinkTime < 90)
		{
			if (self->thinkTime % 3 == 0)
			{
				self->flags ^= NO_DRAW;
			}

			if (self->thinkTime <= 0)
			{
				self->inUse = FALSE;
			}
		}
	}
}

static void creditsHeadMove()
{
	self->face = self->head->face;

	setEntityAnimation(self, self->head->animationName);

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;

	self->mental++;

	if (self->mental != 0 && (self->mental % 300) == 0)
	{
		self->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		self->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		self->creditsAction = &fallOff;

		self->thinkTime = 360;
	}
}
