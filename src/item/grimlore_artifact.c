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
#include "../system/error.h"
#include "../system/properties.h"
#include "key_items.h"

extern Entity *self, player;
extern Game game;

static void reflectionShieldInit(int);
static void protectionShieldInit(int);
static void touch(Entity *);
static void protectionShieldWait(void);
static void reflectionShieldWait(void);
static void bindWait(void);
static void throwBindArtifact(int);
static void bindTouch(Entity *);

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

Entity *addBindArtifact(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Bind Artifact");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;

	e->touch = &keyItemTouch;

	e->fallout = &keyItemFallout;

	e->activate = &throwBindArtifact;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void reflectionShieldInit(int val)
{
	Entity *e;

	if (game.status == IN_GAME && self->thinkTime <= 0 && player.element != WATER)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Reflection Shield");
		}

		loadProperties("edgar/edgar_reflection_shield", e);

		e->action = &reflectionShieldWait;

		e->draw = &drawLoopingAnimationToMap;

		e->touch = &touch;

		e->health = self->maxThinkTime;

		self->thinkTime = self->maxThinkTime;
	}
}

static void protectionShieldInit(int val)
{
	Entity *e;

	if (game.status == IN_GAME && self->thinkTime <= 0 && player.element != WATER)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Protection Shield");
		}

		loadProperties("edgar/edgar_protection_shield", e);

		e->action = &protectionShieldWait;

		e->draw = &drawLoopingAnimationToMap;

		e->touch = &touch;

		e->head = self;

		self->thinkTime = 9999999;
	}
}

static void reflectionShieldWait()
{
	float radians;

	self->x = player.x + player.w / 2 - self->w / 2;
	self->y = player.y + player.h / 2 - self->h / 2;

	self->thinkTime += 5;

	radians = DEG_TO_RAD(self->thinkTime);

	self->alpha = 128 + (64 * cos(radians));

	self->health--;

	if (self->health <= 60 && self->health % 3 == 0)
	{
		self->flags ^= NO_DRAW;
	}

	if (self->health <= 0)
	{
		self->inUse = FALSE;
	}
}

static void protectionShieldWait()
{
	float radians;

	self->x = player.x + player.w / 2 - self->w / 2;
	self->y = player.y + player.h / 2 - self->h / 2;

	self->thinkTime += 5;

	radians = DEG_TO_RAD(self->thinkTime);

	self->alpha = 128 + (64 * cos(radians));

	player.element = FIRE;

	if (player.dirX != 0 || player.dirY != 0)
	{
		player.element = NO_ELEMENT;

		self->head->thinkTime = 0;

		self->inUse = FALSE;
	}
}

static void touch(Entity *other)
{

}

static void throwBindArtifact(int val)
{
	Entity *e;

	if (game.status == IN_GAME && player.element != WATER)
	{
		setEntityAnimation(self, "WALK");

		self->active = TRUE;

		e = addEntity(*self, player.x + (player.face == RIGHT ? player.w : 0), player.y);

		e->thinkTime = 120;

		e->flags |= DO_NOT_PERSIST|LIMIT_TO_SCREEN;

		e->touch = &bindTouch;

		e->action = &bindWait;

		e->dirX = player.face == LEFT ? -8 : 8;

		e->dirY = ITEM_JUMP_HEIGHT;

		e->fallout = &entityDieNoDrop;

		playSoundToMap("sound/common/throw", -1, player.x, player.y, 0);

		self->inUse = FALSE;
	}
}

static void bindWait()
{
	long onGround;

	onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (landedOnGround(onGround) == TRUE)
	{
		self->dirX = 0;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}
}

static void bindTouch(Entity *other)
{
	if (strcmpignorecase(other->objectiveName, self->requires) == 0)
	{
		if (other->health <= 0)
		{
			other->endY = -100;
		}

		else
		{
			other->endY = -99;

			other->health = other->maxHealth;
		}

		self->inUse = FALSE;
	}

	else if (other->type == PLAYER && self->thinkTime <= 0)
	{
		keyItemTouch(other);
	}
}
