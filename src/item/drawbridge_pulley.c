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
#include "../event/script.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void entityWait(void);
static void touch(Entity *);
static void init(void);
static void unravel(void);
static void activate(int);
static void takeDamage(Entity *, int);
static int drawChain(void);
static void chainWait(void);
static void addChain(void);

Entity *addDrawbridgePulley(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Drawbridge Pulley");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;

	e->touch = &touch;

	e->activate = &activate;

	e->takeDamage = &takeDamage;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	e->thinkTime = 0;

	return e;
}

static void init()
{
	self->takeDamage = self->active == TRUE ? takeDamage : NULL;

	self->action = &entityWait;

	addChain();
}

static void entityWait()
{
	checkToMap(self);
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && self->active == TRUE)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
	}

	else
	{
		entityTouch(other);
	}
}

static void activate(int val)
{
	if (self->active == TRUE)
	{
		runScript(self->requires);
	}
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		if (strcmpignorecase(other->name, "weapon/pickaxe") == 0)
		{
			self->health--;

			if (self->health > 0)
			{
				setCustomAction(self, &flashWhite, 6, 0, 0);
			}

			else
			{
				self->targetX = playSoundToMap("sound/item/chain", -1, self->x, self->y, -1);

				self->health = 0;

				self->touch = NULL;

				self->active = FALSE;

				self->thinkTime = 300;

				self->action = &unravel;

				setEntityAnimation(self, "WALK");

				setEntityAnimation(self->target, "WALK");
			}
		}

		else
		{
			playSoundToMap("sound/common/dink", -1, self->x, self->y, 0);

			if (other->reactToBlock != NULL)
			{
				temp = self;

				self = other;

				self->reactToBlock(temp);

				self = temp;
			}

			damage = 0;

			if (other->type != PROJECTILE && prand() % 10 == 0)
			{
				setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
			}
		}

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
	}
}

static void unravel()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->frameSpeed = 0;

		setEntityAnimation(self->target, "STAND");

		self->action = &entityWait;

		stopSound(self->targetX);

		runScript(self->requires);
	}
}

static void addChain()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Drawbridge Pulley Chain");
	}

	loadProperties("item/drawbridge_pulley_chain", e);

	e->x = self->x;
	e->y = self->y;

	e->face = RIGHT;

	e->action = &chainWait;

	e->draw = &drawChain;

	setEntityAnimation(e, "STAND");

	self->target = e;

	e->startX = self->startX;
	e->startY = self->startY;

	e->endX = self->endX;
	e->endY = self->endY;
}

static void chainWait()
{

}

static int drawChain()
{
	self->x = self->startX;
	self->y = self->startY;

	drawLoopingAnimationToMap();

	while (self->x > self->endX && self->y > self->endY)
	{
		drawSpriteToMap();

		self->x -= self->offsetX;
		self->y -= self->offsetY;
	}

	return TRUE;
}
