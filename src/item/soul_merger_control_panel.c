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

#include "../collisions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../graphics/graphics.h"
#include "../hud.h"
#include "../inventory.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;
extern Input input;

static void entityWait(void);
static void touch(Entity *);
static void activate(int);
static int energyBarDraw(void);
static void energyBarWait(void);
static void init(void);
static void reprogram(void);

Entity *addSoulMergerControlPanel(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Soul Merger Control Panel");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;
	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	if (self->damage == 1 && self->mental == 0 && self->thinkTime == 0)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to reprogram the Soul Merger"));
	}
}

static void activate(int val)
{
	if (getInventoryItemByObjectiveName("Spanner") == NULL)
	{
		setInfoBoxMessage(120, 255, 255, 255, _("Spanner is required"));
	}

	else
	{
		self->action = &reprogram;

		self->touch = NULL;

		self->activate = NULL;

		setPlayerLocked(TRUE);
	}
}

static void reprogram()
{
	Entity *temp;

	if (input.interact == 1 || isPlayerLocked() == FALSE || self->health <= 0)
	{
		self->action = &entityWait;

		self->activate = &activate;

		self->touch = &touch;

		setPlayerLocked(FALSE);

		input.interact = 0;
	}

	else
	{
		self->health--;

		if (self->health <= 0)
		{
			self->activate = NULL;

			self->health = 0;

			self->mental = 1;

			self->target->mental = 1 - self->target->mental;

			temp = self;

			self = self->target;

			self->activate(-1);

			self = temp;
		}

		self->thinkTime = 5;

		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to Cancel"));
	}
}

static void init()
{
	Entity *e;

	if (self->mental == 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Soul Merger Control Panel Energy Bar");
		}

		loadProperties("boss/awesome_boss_energy_bar", e);

		e->action = &energyBarWait;

		e->draw = &energyBarDraw;

		e->type = ENEMY;

		e->head = self;

		setEntityAnimation(e, "STAND");

		self->action = &entityWait;

		self->target = getEntityByObjectiveName(self->requires);

		if (self->target == NULL)
		{
			showErrorAndExit("Control Panel cannot find Soul Merger %s", self->requires);
		}

		self->activate = &activate;
	}
}

static void energyBarWait()
{
	self->x = self->head->x - (self->w - self->head->w) / 2;
	self->y = self->head->y - self->head->h;

	if (self->health < self->head->health)
	{
		self->health += (self->head->health / 100);

		if (self->health > self->head->health)
		{
			self->health = self->head->health;
		}
	}

	else if (self->head->health < self->health)
	{
		self->health -= 3;

		if (self->health < self->head->health)
		{
			self->health = self->head->health;
		}
	}
}

static int energyBarDraw()
{
	int width;
	float percentage;

	if (self->head->thinkTime != 0)
	{
		drawLoopingAnimationToMap();

		percentage = self->health;
		percentage /= self->head->maxHealth;

		width = self->w - 2;

		width *= percentage;

		drawBoxToMap(self->x + 1, self->y + 1, width, self->h - 2, 0, 220, 0);
	}

	return TRUE;
}
