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
#include "../entity.h"
#include "../graphics/animation.h"
#include "../inventory.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "item.h"
#include "key_items.h"

extern Entity *self;
extern Entity player;
extern Game game;

static void respawn(void);
static void noTouch(Entity *);

Entity *addPermanentItem(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add item %s", name);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &doNothing;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->fallout = &itemFallout;

	e->creditsAction = &doNothing;

	if (e->type == HEALTH)
	{
		e->touch = &healthTouch;
	}

	else if (e->type == WEAPON || e->type == SHIELD)
	{
		e->touch = &keyItemTouch;

		e->fallout = &keyItemFallout;

		if (e->type == WEAPON)
		{
			if (strcmpignorecase("weapon/normal_arrow", e->name) == 0)
			{
				e->activate = &setBowAmmo;
			}

			else if (strcmpignorecase("weapon/flaming_arrow", e->name) == 0)
			{
				e->activate = &setBowAmmo;
			}

			else
			{
				e->activate = &setPlayerWeapon;
			}
		}

		else
		{
			e->activate = &setPlayerShield;
		}
	}

	else if ((e->flags & PUSHABLE) || (e->flags & OBSTACLE))
	{
		e->touch = &pushEntity;

		e->frameSpeed = 0;
	}

	else if (e->flags & NO_DRAW)
	{
		e->touch = &noTouch;
	}

	else
	{
		e->touch = &keyItemTouch;
	}

	setEntityAnimation(e, "STAND");

	return e;
}

Entity *addTemporaryItem(char *name, int x, int y, int face, float dirX, float dirY)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add item %s", name);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	if (e->thinkTime <= 0)
	{
		showErrorAndExit("No valid thinkTime defined for %s", name);
	}

	e->dirX = dirX;
	e->dirY = dirY;

	e->face = face;

	e->action = &generalItemAction;
	e->draw = &drawLoopingAnimationToMap;

	e->fallout = &entityDieNoDrop;

	e->touch = &noTouch;

	e->creditsAction = &generalItemAction;

	switch (e->type)
	{
		case HEALTH:
			e->touch = &healthTouch;

			e->flags |= DO_NOT_PERSIST;
		break;

		case WEAPON:
			e->touch = &keyItemTouch;

			e->activate = &setBowAmmo;

			e->flags |= DO_NOT_PERSIST;
		break;

		default:
			if (e->element == LIGHTNING)
			{
				e->touch = &lightningChargeTouch;

				e->flags |= DO_NOT_PERSIST;
			}

			else
			{
				e->type = TEMP_ITEM;
			}
		break;
	}

	setEntityAnimationByID(e, 0);

	return e;
}

Entity *dropCollectableItem(char *name, int x, int y, int face)
{
	Entity *e = addTemporaryItem(name, x, y, face, 0, ITEM_JUMP_HEIGHT);

	e->type = ITEM;

	e->touch = &keyItemTouch;

	return e;
}

void dropRandomItem(int x, int y)
{
	Entity *e;

	if (prand() % 3 == 0)
	{
		addTemporaryItem("item/heart", x, y, RIGHT, 0, ITEM_JUMP_HEIGHT);
	}

	if (hasBow() == TRUE && prand() % 5 == 0)
	{
		e = addTemporaryItem("weapon/normal_arrow", x, y, RIGHT, 0, ITEM_JUMP_HEIGHT);

		e->health = 1 + (prand() % 3);
	}

	if (hasLightningSword() == TRUE && prand() % 5 == 0)
	{
		e = addTemporaryItem("item/lightning_charge", x, y, RIGHT, 0, ITEM_JUMP_HEIGHT);

		e->health = 1 + (prand() % 3);
	}
}

void generalItemAction()
{
	self->thinkTime--;

	if ((self->flags & ON_GROUND) || (self->standingOn != NULL))
	{
		self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
	}

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

	checkToMap(self);

	self->standingOn = NULL;
}

void healthTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		other->health += self->health;

		if (other->health > other->maxHealth)
		{
			other->health = other->maxHealth;
		}

		self->inUse = FALSE;
	}
}

void lightningChargeTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		addChargesToWeapon();

		self->inUse = FALSE;
	}
}

void throwItem(int val)
{
	Entity *e;

	if (game.status == IN_GAME && self->thinkTime <= 0 && !(player.flags & BLOCKING) && player.element != WATER)
	{
		e = addProjectile(self->name, &player, player.x + (player.face == RIGHT ? player.w : 0), player.y, player.face == LEFT ? -self->speed : self->speed, 0);

		e->y = player.y + (player.h - e->h) / 2;

		e->type = PROJECTILE;

		e->flags |= FLY;

		e->touch = &entityTouch;

		e->damage = self->damage;

		e->parent = &player;

		e->thinkTime = 600;

		self->health--;

		self->thinkTime = 15;

		if (self->health <= 0)
		{
			self->inUse = FALSE;
		}

		playSoundToMap("sound/common/throw", EDGAR_CHANNEL, player.x, player.y, 0);
	}
}

void itemFallout()
{
	self->thinkTime = 120;

	self->action = &respawn;
}

static void respawn()
{
	self->thinkTime--;

	checkToMap(self);

	if (self->thinkTime <= 0)
	{
		self->x = self->startX;
		self->y = self->startY;

		setCustomAction(self, &invulnerable, 60, 0, 0);

		self->action = &doNothing;
	}
}

static void noTouch(Entity *other)
{

}
