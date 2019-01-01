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
#include "../entity.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self, player;

static void entityWait(void);
static void touch(Entity *);
static void activate(int);
static void init(void);
static void addDoor(void);
static void doorWait(void);
static void doorClose(void);
static void doorOpen(void);

Entity *addSoulMerger(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Soul Merger");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;
	e->touch = &touch;
	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	addDoor();

	setEntityAnimation(self, self->mental == 0 ? "STAND" : "WALK");

	self->action = &entityWait;
}

static void entityWait()
{
	if (self->health == 3)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->thinkTime = 120;

			setEntityAnimation(self, self->mental == 0 ? "STAND" : "WALK");

			self->health = 4;
		}
	}

	else if (self->health == 4)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->health = 5;
		}
	}

	if (self->target != NULL)
	{
		self->target->flags |= INVULNERABLE;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	if (self->health == 0 && self->active == TRUE)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to enter the Soul Merger"));
	}
}

static void activate(int val)
{
	Entity *other;

	if (self->health == 0)
	{
		if (val == 0)
		{
			if (self->active == TRUE)
			{
				other = getEntityByObjectiveName(self->requires);

				if (other == NULL)
				{
					showErrorAndExit("Soul Merger could not find target chamber %s", self->requires);
				}

				if (other->mental == self->mental)
				{
					setInfoBoxMessage(120, 255, 255, 255, _("An IN Chamber and an OUT Chamber are required"));
				}

				else
				{
					self->target = &player;

					setPlayerLocked(TRUE);
				}
			}
		}

		else if (val == -1)
		{
			setEntityAnimation(self, self->mental == 0 ? "STAND" : "WALK");
		}

		else
		{
			self->target = getEntityByObjectiveName("EVIL_EDGAR_2");
		}

		if (self->target != NULL)
		{
			self->target->x = self->x + self->w / 2 - self->target->w / 2;

			self->health = 1;
		}
	}
}

static void addDoor()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Soul Merger Door");
	}

	loadProperties("item/soul_merger_door", e);

	setEntityAnimation(e, "STAND");

	e->x = self->x + self->w / 2 - e->w / 2 - e->w;
	e->y = self->y + self->h - e->h;

	e->startX = e->x;

	e->endX = self->x + self->w / 2 - e->w / 2;

	e->type = KEY_ITEM;

	e->action = &doorWait;

	e->draw = &drawLoopingAnimationToMap;

	e->head = self;
}

static void doorWait()
{
	if (self->head->health == 1)
	{
		self->layer = FOREGROUND_LAYER;

		self->action = &doorClose;
	}

	else if (self->head->health == 5)
	{
		self->layer = FOREGROUND_LAYER;

		self->action = &doorOpen;
	}

	checkToMap(self);
}

static void doorClose()
{
	if (fabs(self->endX - self->x) <= fabs(self->dirX))
	{
		self->layer = BACKGROUND_LAYER;

		self->x = self->endX;

		self->dirX = 0;

		self->head->health = 2;

		self->action = &doorWait;

		self->head->target->flags |= NO_DRAW;

		playSoundToMap("sound/common/door", -1, self->x, self->y, 0);
	}

	else
	{
		self->dirX = self->endX < self->x ? -2 : 2;
	}

	checkToMap(self);
}

static void doorOpen()
{
	if (fabs(self->startX - self->x) <= fabs(self->dirX))
	{
		self->x = self->startX;

		self->dirX = 0;

		self->head->health = 6;

		if (self->target == &player)
		{
			setPlayerLocked(FALSE);
		}

		self->head->target = NULL;

		self->action = &doorWait;

		self->layer = BACKGROUND_LAYER;

		playSoundToMap("sound/common/door", -1, self->x, self->y, 0);
	}

	else
	{
		self->dirX = self->startX < self->x ? -2 : 2;
	}

	checkToMap(self);
}
