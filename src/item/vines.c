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

extern Entity *self;

static void entityWait(void);
static void touch(Entity *);
static void takeDamage(Entity *, int);
static void fireWait(void);

Entity *addVines(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Vines");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &entityWait;
	e->touch = &touch;
	e->takeDamage = &takeDamage;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	int frameCount = getFrameCount(self);
	Entity *e;

	if (self->active == TRUE)
	{
		self->thinkTime--;

		if (self->thinkTime % 120 == 0)
		{
			self->currentFrame++;

			if (self->currentFrame >= frameCount)
			{
				self->currentFrame = frameCount - 1;
			}
		}

		else if (self->thinkTime % 5 == 0)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add the Fire");
			}

			loadProperties("boss/phoenix_die_fire", e);

			setEntityAnimation(e, "STAND");

			e->x = self->x + prand() % self->w;
			e->y = self->y + self->h - e->h;

			e->action = &fireWait;

			e->draw = &drawLoopingAnimationToMap;

			e->type = ENEMY;

			e->flags |= DO_NOT_PERSIST|FLY;

			e->thinkTime = 30;

			e->health = 0;

			e->maxHealth = 3 + prand() % 3;

			e->mental = 1;

			e->head = self;
		}

		if (self->thinkTime <= 0)
		{
			self->inUse = FALSE;
		}
	}

	else
	{
		self->frameSpeed = 0;

		self->thinkTime = frameCount * 120;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	if (strcmpignorecase(other->name, "weapon/flaming_arrow") == 0)
	{
		self->active = TRUE;

		other->inUse = FALSE;
	}

	else if ((other->flags & ATTACKING) && !(self->flags & INVULNERABLE))
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

static void fireWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->health += self->mental;

		if (self->health == self->maxHealth)
		{
			self->maxHealth = 5;

			self->thinkTime = 60;

			self->mental *= -1;
		}

		else if (self->health < 0)
		{
			self->head->maxThinkTime--;

			self->inUse = FALSE;

			self->health = 0;
		}

		else
		{
			self->thinkTime = 20;
		}

		setEntityAnimationByID(self, self->health);
	}

	checkToMap(self);
}
