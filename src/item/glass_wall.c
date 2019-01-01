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
#include "../event/global_trigger.h"
#include "../event/trigger.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "item.h"

extern Entity *self;

static void activate(int);
static void takeDamage(Entity *, int);
static void glassWait(void);
static void die(void);
static void respawnWait(void);
static void respawn(void);
static void horizontalGlassWait(void);

Entity *addGlassWall(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Glass Wall");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = strcmpignorecase(name, "item/horizontal_glass_wall") == 0 ? &horizontalGlassWait : &glassWait;
	e->touch = &pushEntity;
	e->activate = &activate;
	e->takeDamage = &takeDamage;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void activate(int val)
{
	if (val == 100 && self->touch != NULL)
	{
		self->active = TRUE;
	}
}

static void glassWait()
{
	if (self->active == TRUE)
	{
		if ((self->thinkTime % 60) == 0)
		{
			self->mental++;

			setEntityAnimationByID(self, self->mental);

			playSoundToMap("sound/item/crack", -1, self->x, self->y, 0);
		}

		self->thinkTime--;

		if (self->mental == 5)
		{
			playSoundToMap("sound/common/shatter", -1, self->x, self->y, 0);

			self->action = &die;
		}
	}

	checkToMap(self);
}

static void horizontalGlassWait()
{
	if (self->active == TRUE)
	{
		if (self->mental < 3)
		{
			self->mental++;

			setEntityAnimationByID(self, self->mental);

			self->active = FALSE;

			playSoundToMap("sound/item/crack", -1, self->x, self->y, 0);

			if (self->mental == 3)
			{
				STRNCPY(self->objectiveName, self->requires, sizeof(self->objectiveName));

				STRNCPY(self->requires, "BOSS_TUNING_FORK", sizeof(self->requires));
			}
		}

		else
		{
			self->thinkTime++;

			if ((self->thinkTime % 180) == 0)
			{
				self->mental++;

				setEntityAnimationByID(self, self->mental);

				playSoundToMap("sound/item/crack", -1, self->x, self->y, 0);
			}

			if (self->mental == 4)
			{
				playSoundToMap("sound/common/shatter", -1, self->x, self->y, 0);

				self->action = &die;
			}
		}
	}

	checkToMap(self);
}

static void die()
{
	int i;
	Entity *e;

	if (strcmpignorecase(self->name, "item/horizontal_glass_wall") == 0)
	{
		for (i=0;i<7;i++)
		{
			e = addTemporaryItem("item/horizontal_glass_wall_piece", self->x, self->y, RIGHT, 0, 0);

			e->x += self->w / 2 - e->w / 2;
			e->y += self->h / 2 - e->h / 2;

			e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

			setEntityAnimationByID(e, i);

			e->thinkTime = 60 + (prand() % 60);
		}

		self->flags |= NO_DRAW;

		self->touch = NULL;

		self->action = &respawnWait;

		self->thinkTime = 300;
	}

	else
	{
		for (i=0;i<7;i++)
		{
			e = addTemporaryItem("item/glass_wall_piece", self->x, self->y, RIGHT, 0, 0);

			e->x += self->w / 2 - e->w / 2;
			e->y += self->h / 2 - e->h / 2;

			e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

			setEntityAnimationByID(e, i);

			e->thinkTime = 60 + (prand() % 60);
		}

		self->inUse = FALSE;

		fireTrigger(self->objectiveName);

		fireGlobalTrigger(self->objectiveName);
	}
}

static void respawnWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime = 60;

		self->action = &respawn;
	}
}

static void respawn()
{
	self->thinkTime--;

	if (self->thinkTime % 3 == 0)
	{
		self->flags ^= NO_DRAW;
	}

	if (self->thinkTime <= 0)
	{
		self->flags &= ~NO_DRAW;

		self->touch = &pushEntity;

		self->mental = 0;

		self->action = &horizontalGlassWait;

		self->thinkTime = self->maxThinkTime;

		self->active = FALSE;

		STRNCPY(self->requires, self->objectiveName, sizeof(self->requires));
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
