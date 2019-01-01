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
#include "../event/script.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../item/item.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void entityWait(void);
static void blend(void);
static void blendFinish(void);
static void touch(Entity *);
static void init(void);
static void activate(int);
static void handleWait(void);
static void switchWait(void);
static Entity *addComponent(char *);

Entity *addBlendingMachine(int x, int y, char *name)
{
	Entity *e = getFreeEntity();
	Entity *component;

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Blending Machine");
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

	component = addComponent("item/blender_wheel");

	component->action = &handleWait;

	component->face = RIGHT;

	component->parent = e;

	component->x = x;

	component->y = y;

	component = addComponent("item/blender_switch");

	component->action = &switchWait;

	component->face = RIGHT;

	component->frameSpeed = 0;

	component->parent = e;

	component->x = x;

	component->y = y;

	return e;
}

static void entityWait()
{
	if (self->health == 6)
	{
		self->thinkTime = 600;

		self->mental = playSoundToMap("sound/item/blender", -1, self->x, self->y, -1);

		self->action = &blend;

		self->health = -1;
	}

	checkToMap(self);
}

static void blend()
{
	self->thinkTime--;

	switch (self->thinkTime)
	{
		case 420:
			setEntityAnimation(self, "ATTACK_2");
		break;

		case 300:
			setEntityAnimation(self, "ATTACK_3");
		break;
	}

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 90;

		setEntityAnimation(self, "ATTACK_4");

		self->action = &blendFinish;
	}

	checkToMap(self);
}

static void blendFinish()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->health = 0;

		stopSound(self->mental);

		playSoundToMap("sound/item/ping", -1, self->x, self->y, 0);

		e = addPermanentItem(self->requires, self->x + self->w / 2, self->y + self->h / 2);

		e->x -= e->w / 2;

		e->dirY = ITEM_JUMP_HEIGHT;

		self->action = &entityWait;

		setEntityAnimation(self, "ATTACK_1");
	}
}

static void touch(Entity *other)
{
	if (other->type == PLAYER)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
	}
}

static void activate(int val)
{
	if (self->active == TRUE)
	{
		runScript("blender");
	}

	else
	{
		setInfoBoxMessage(60, 255, 255, 255, _("This machine is not active"));
	}
}

static void init()
{
	setEntityAnimation(self, self->active == FALSE ? "STAND" : "ATTACK_1");

	self->action = self->thinkTime > 0 ? &blend : &entityWait;
}

static Entity *addComponent(char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Blending Machine Component");
	}

	loadProperties(name, e);

	e->type = KEY_ITEM;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void handleWait()
{
	if (self->parent->health != 0)
	{
		if (self->parent->thinkTime == 420)
		{
			setEntityAnimation(self, "WALK");
		}
	}

	else
	{
		setEntityAnimation(self, "STAND");
	}
}

static void switchWait()
{
	if (self->parent->health != 0)
	{
		switch (self->parent->thinkTime)
		{
			case 300:
			case 240:
				self->currentFrame++;

				setFrameData(self);

				playSoundToMap("sound/common/switch", -1, self->x, self->y, 0);
			break;

			case 150:
			case 120:
				self->currentFrame--;

				setFrameData(self);

				playSoundToMap("sound/common/switch", -1, self->x, self->y, 0);
			break;
		}
	}

	else
	{
		setEntityAnimation(self, "STAND");
	}
}
