/*
Copyright (C) 2009 Parallel Realities

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
#include "../entity.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../system/properties.h"
#include "../player.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "../system/random.h"
#include "../inventory.h"
#include "../projectile.h"
#include "../custom_actions.h"
#include "../system/error.h"

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

	setEntityAnimation(e, STAND);

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

	switch (e->type)
	{
		case HEALTH:
			e->touch = &healthTouch;
		break;

		case WEAPON:
			e->touch = &keyItemTouch;

			e->activate = &setBowAmmo;
		break;

		default:
			e->type = TEMP_ITEM;
		break;
	}

	setEntityAnimation(e, STAND);

	return e;
}

Entity *dropCollectableItem(char *name, int x, int y, int face)
{
	Entity *e = addTemporaryItem(name, x, y, face, 0, ITEM_JUMP_HEIGHT);

	e->touch = &keyItemTouch;

	return e;
}

void dropRandomItem(int x, int y)
{
	Entity *e;
	
	if (getInventoryItem(_("Bow")) != NULL)
	{
		if (prand() % 3 == 0)
		{
			addTemporaryItem("item/heart", x, y, RIGHT, 0, ITEM_JUMP_HEIGHT);
		}
		
		if (prand() % 3 == 0)
		{
			e = addTemporaryItem("weapon/normal_arrow", x, y, RIGHT, 0, ITEM_JUMP_HEIGHT);

			e->health = 1 + (prand() % 3);
		}
	}

	else if (prand() % 3 == 0)
	{
		addTemporaryItem("item/heart", x, y, RIGHT, 0, ITEM_JUMP_HEIGHT);
	}
}

void generalItemAction()
{
	self->thinkTime--;

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
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

void throwItem(int val)
{
	Entity *e;
	
	if (game.status == IN_GAME)
	{
		e = addProjectile(self->name, &player, player.x + (player.face == RIGHT ? player.w : 0), player.y + player.h / 2, player.face == LEFT ? -self->speed : self->speed, 0);

		e->type = PROJECTILE;

		e->flags |= FLY;

		e->touch = &entityTouch;

		e->damage = self->damage;

		e->parent = &player;

		e->thinkTime = 600;
		
		self->health--;

		if (self->health <= 0)
		{
			self->inUse = FALSE;
		}
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

		setCustomAction(self, &invulnerable, 60, 0);

		self->action = &doNothing;
	}
}

static void noTouch(Entity *other)
{

}
