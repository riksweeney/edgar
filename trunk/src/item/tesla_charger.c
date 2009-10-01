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
#include "../system/properties.h"
#include "../entity.h"
#include "../hud.h"
#include "../collisions.h"
#include "../inventory.h"
#include "../audio/audio.h"
#include "tesla_pack.h"

extern Entity *self;

static void init(void);
static void wait(void);
static void recharge(void);
static void setChargeState(void);
static void activate(int);
static void touch(Entity *);

Entity *addTeslaCharger(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Tesla Charger\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;

	e->touch = &touch;

	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	setChargeState();
}

static void recharge()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->health++;

		setChargeState();

		playSoundToMap("sound/item/charge_beep.ogg", -1, self->x, self->y, 0);

		self->thinkTime = 180;
	}

	checkToMap(self);
}

static void wait()
{
	checkToMap(self);
}

static void setChargeState()
{
	switch (self->health)
	{
		case 0:
			setEntityAnimation(self, STAND);
			self->action = &recharge;
		break;

		case 1:
			setEntityAnimation(self, WALK);
			self->action = &recharge;
		break;

		case 2:
			setEntityAnimation(self, JUMP);
			self->action = &recharge;
		break;

		case 3:
			setEntityAnimation(self, PAIN);
			self->action = &wait;
		break;

		default:
			setEntityAnimation(self, DIE);
			self->action = &wait;
		break;
	}
}

static void activate(int val)
{
	Entity *e;

	if (self->health == 3)
	{
		printf("Adding pack to inventory\n");

		e = addTeslaPack(0, 0, "item/tesla_pack_full");

		addToInventory(e);

		self->health = -1;
	}

	else
	{
		e = getInventoryItem(_("Tesla Pack"));

		if (e != NULL && e->health == 0)
		{
			removeInventoryItem(e->objectiveName);

			self->health = 0;

			self->thinkTime = 180;
		}
	}

	setChargeState();
}

static void touch(Entity *other)
{
	Entity *e;

	if (other->type == PLAYER)
	{
		if (self->health == 3)
		{
			setInfoBoxMessage(0, _("Press Action to retrieve Tesla Pack"));
		}

		else if (self->health == -1)
		{
			e = getInventoryItem(_("Tesla Pack"));

			if (e != NULL && e->health == 0)
			{
				setInfoBoxMessage(0, _("Press Action to replace Tesla Pack"));
			}
		}
	}
}
