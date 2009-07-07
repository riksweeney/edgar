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
#include "../custom_actions.h"

extern Entity *self;
extern Entity player, playerShield, playerWeapon;

static void respawn(void);
static void itemFallout(void);
static void noTouch(Entity *);

Entity *addPermanentItem(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add item %s\n", name);

		exit(1);
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
		printf("No free slots to add item %s\n", name);

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	if (e->thinkTime <= 0)
	{
		printf("No valid thinkTime defined for %s\n", name);

		exit(1);
	}

	e->dirX = dirX;
	e->dirY = dirY;

	e->face = face;

	e->action = &generalItemAction;
	e->draw = &drawLoopingAnimationToMap;

	e->fallout = &entityDie;

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

void dropRandomItem(int x, int y)
{
	if (prand() % 3 == 0)
	{
		if (getInventoryItem("weapon/bow") != NULL)
		{
			if (prand() % 2 == 0)
			{
				addTemporaryItem("weapon/arrow", x, y, RIGHT, 0, ITEM_JUMP_HEIGHT);
			}

			if (prand() % 2 == 0)
			{
				addTemporaryItem("item/heart", x, y, RIGHT, 0, ITEM_JUMP_HEIGHT);
			}
		}

		else
		{
			addTemporaryItem("item/heart", x, y, RIGHT, 0, ITEM_JUMP_HEIGHT);
		}
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

void dropItem(Entity *e)
{
	if (e->type == SHIELD)
	{
		if (strcmpignorecase(playerShield.name, e->name) == 0)
		{
			playerShield.name[0] = '\0';

			playerShield.inUse = FALSE;
		}
	}

	else if (e->type == WEAPON)
	{
		if (strcmpignorecase(playerWeapon.name, e->name) == 0)
		{
			playerWeapon.name[0] = '\0';

			playerWeapon.inUse = FALSE;
		}
	}

	e->dirY = ITEM_JUMP_HEIGHT;

	e->action = &doNothing;

	setCustomAction(e, &invulnerable, 180, 0);

	addEntity(*e, player.x, player.y);
}

void throwItem(int val)
{
	Entity *e;

	e = addTemporaryItem(self->name, player.x + (player.face == RIGHT ? player.w : 0), player.y + player.h / 2, player.face, player.face == LEFT ? -7 : 7, 0);

	self->inUse = FALSE;

	e->type = PROJECTILE;

	e->flags |= FLY;

	e->touch = self->touch;

	e->damage = self->damage;

	e->parent = &player;

	e->thinkTime = 600;
}

static void itemFallout()
{
	self->thinkTime = 300;

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

		setCustomAction(self, &invulnerable, 180, 0);

		self->action = &doNothing;
	}
}

static void noTouch(Entity *other)
{

}
