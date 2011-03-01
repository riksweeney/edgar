/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "../graphics/decoration.h"
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "key_items.h"
#include "item.h"
#include "../hud.h"
#include "../inventory.h"
#include "../custom_actions.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../collisions.h"
#include "../event/script.h"
#include "../system/error.h"
#include "../system/random.h"

extern Entity *self, player;
extern Game game;

static void reflectionShieldInit(int);
static void protectionShieldInit(int);
static void shieldWait(void);
static void touch(Entity *);

Entity *addReflectionArtifact(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Reflection Artifact");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;

	e->touch = &keyItemTouch;

	e->fallout = &keyItemFallout;

	e->activate = &reflectionShieldInit;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

Entity *addProtectionArtifact(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Protection Artifact");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;

	e->touch = &keyItemTouch;

	e->fallout = &keyItemFallout;

	e->activate = &protectionShieldInit;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void reflectionShieldInit(int val)
{
	Entity *e;

	if (game.status == IN_GAME)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Reflection Shield");
		}

		loadProperties("edgar/edgar_reflection_shield", e);

		e->action = &shieldWait;

		e->draw = &drawLoopingAnimationToMap;

		e->touch = &touch;
	}
}

static void protectionShieldInit(int val)
{
	Entity *e;

	if (game.status == IN_GAME)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Protection Shield");
		}

		loadProperties("edgar/edgar_protection_shield", e);

		e->action = &shieldWait;

		e->draw = &drawLoopingAnimationToMap;

		e->touch = &touch;
	}
}

static void shieldWait()
{
	float radians;

	self->x = player.x + player.w / 2 - self->w / 2;
	self->y = player.y + player.h / 2 - self->h / 2;

	self->thinkTime += 5;

	radians = DEG_TO_RAD(self->thinkTime);

	self->alpha = 128 + (64 * cos(radians));

	if (player.dirX != 0 || player.dirY != 0)
	{
		self->inUse = FALSE;
	}
}

static void touch(Entity *other)
{

}
