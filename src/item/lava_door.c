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
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "item.h"

extern Entity *self;

static void touch(Entity *);
static void entityWait(void);
static void init(void);
static void die(void);
static void takeDamage(Entity *, int);

Entity *addLavaDoor(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Lava Door");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->takeDamage = &takeDamage;
	e->action = &init;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	setEntityAnimationByID(self, self->health);

	self->action = &entityWait;
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->health--;

		if (self->health <= 0)
		{
			self->health = 0;
		}

		self->thinkTime = 600;

		setEntityAnimationByID(self, self->health);
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	if (other->type == PROJECTILE && other->element == FIRE)
	{
		self->health++;

		if (self->health == 7)
		{
			setInfoBoxMessage(120, 255, 255, 255, _("One blow from the pickaxe should shatter it"));
		}

		else if (self->health > 7)
		{
			self->health = 7;
		}

		setEntityAnimationByID(self, self->health);

		self->thinkTime = 600;

		other->inUse = FALSE;
	}

	else if (((other->flags & ATTACKING) || other->type == PROJECTILE) && !(self->flags & INVULNERABLE))
	{
		takeDamage(other, other->damage);
	}

	else
	{
		pushEntity(other);
	}
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->health < 7)
	{
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		playSoundToMap("sound/common/dink", -1, self->x, self->y, 0);

		if (other->reactToBlock != NULL)
		{
			temp = self;

			self = other;

			self->reactToBlock(temp);

			self = temp;
		}

		if (other->type != PROJECTILE && prand() % 10 == 0)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
		}

		damage = 0;
	}

	else if (strcmpignorecase(other->name, "weapon/pickaxe") == 0)
	{
		self->action = &die;
	}
}

static void die()
{
	int i;
	Entity *e;

	playSoundToMap("sound/common/shatter", -1, self->x, self->y, 0);

	for (i=0;i<7;i++)
	{
		e = addTemporaryItem("item/lava_door_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += self->w / 2 - e->w / 2;
		e->y += self->h / 2 - e->h / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->thinkTime = 60 + (prand() % 60);
	}

	self->inUse = FALSE;
}
