/*
Copyright (C) 2009-2010 Parallel Realities

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
#include "../entity.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../system/error.h"

extern Entity *self;

static void headDie(void);
static void init(void);
static void headWait(void);
static void lookForPlayer(void);
static void bodyTakeDamage(Entity *, int);
static void headTakeDamage(Entity *, int);
static void alignBodyToHead(void);
static void createBody(void);
static void headChangeDirection(Entity *);
static void bodyWait(void);

Entity *addFlyTrap(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Fly Trap");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Fly Trap Head");
	}

	loadProperties("enemy/fly_trap_head", e);

	e->action = &headWait;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &headDie;
	e->touch = &entityTouch;
	e->takeDamage = &headTakeDamage;
	e->reactToBlock = &headChangeDirection;

	e->head = self;

	createBody();

	self->action = &lookForPlayer;
}

static void lookForPlayer()
{
	moveLeftToRight();
}

static void headDie()
{
	Entity *e;

	e = self;

	self = self->target;

	while (self != NULL)
	{
		self->die();

		self->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		self->dirY = ITEM_JUMP_HEIGHT;

		self = self->target;
	}

	self = e;

	self->die();

	self->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
	self->dirY = ITEM_JUMP_HEIGHT;
}

static void headWait()
{
	self->startX = self->head->x + self->head->w / 2 - self->w / 2;

	self->y = self->head->y - 32;

	self->thinkTime += 5;

	if (self->thinkTime >= 360)
	{
		self->thinkTime = 0;
	}

	self->y = self->startX + cos(DEG_TO_RAD(self->thinkTime)) * 8;

	alignBodyToHead();
}

static void createBody()
{
	int i;
	Entity **body, *head;

	self->x = self->endX;
	self->y = self->endY;

	if (self->mental == 0)
	{
		self->mental = 5;
	}

	body = (Entity **)malloc(self->mental * sizeof(Entity *));

	if (body == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Fly Trap body...\n", self->mental * (int)sizeof(Entity *));
	}

	/* Create in reverse order so that it is drawn correctly */

	resetEntityIndex();

	for (i=self->mental-1;i>=0;i--)
	{
		body[i] = getFreeEntity();

		if (body[i] == NULL)
		{
			showErrorAndExit("No free slots to add a Fly Trap body part");
		}

		loadProperties("enemy/fly_trap_stalk", body[i]);

		body[i]->x = self->x;
		body[i]->y = self->y;

		body[i]->action = &bodyWait;

		body[i]->draw = &drawLoopingAnimationToMap;
		body[i]->touch = &entityTouch;
		body[i]->die = &entityDieNoDrop;
		body[i]->takeDamage = &bodyTakeDamage;

		body[i]->type = ENEMY;

		setEntityAnimation(body[i], STAND);
	}

	/* Recreate the head so that it's on top */

	head = getFreeEntity();

	if (head == NULL)
	{
		showErrorAndExit("No free slots to add a Fly Trap head");
	}

	*head = *self;

	self->inUse = FALSE;

	self = head;

	/* Link the sections */

	for (i=self->mental-1;i>=0;i--)
	{
		if (i == 0)
		{
			self->target = body[i];
		}

		else
		{
			body[i - 1]->target = body[i];
		}

		body[i]->head = self;
	}

	free(body);

	self->action = &headWait;
}

static void alignBodyToHead()
{
	float x, y, partDistanceX, partDistanceY;
	Entity *e;

	x = self->x;
	y = self->y;

	partDistanceX = self->x + self->w - self->target->w;
	partDistanceY = self->y + self->h - self->target->h;

	partDistanceX /= self->mental;
	partDistanceY /= self->mental;

	e = self->target;

	while (e != NULL)
	{
		x += partDistanceX;
		y += partDistanceY;

		e->x = (e->target == NULL ? self->endX : x) + (self->w - e->w) / 2;
		e->y = (e->target == NULL ? self->endY : y);

		e->damage = self->damage;

		e->face = self->face;

		if (self->flags & FLASH)
		{
			e->flags |= FLASH;
		}

		else
		{
			e->flags &= ~FLASH;
		}

		e = e->target;
	}
}

static void bodyTakeDamage(Entity *other, int damage)
{
	Entity *temp = self;

	self = self->head->head; /* Get the head, then the base */

	self->takeDamage(other, damage);

	self = temp;
}

static void headTakeDamage(Entity *other, int damage)
{
	Entity *temp = self;

	self = self->head;

	self->takeDamage(other, damage);

	self = temp;

	if (self->head->health > 0)
	{
		setCustomAction(self, &flashWhite, 6, 0, 0);
	}
}

static void headChangeDirection(Entity *other)
{
	Entity *temp;

	temp = self;

	self->reactToBlock(temp);

	self = temp;

	self->face = self->head->face;
}

static void bodyWait()
{
	checkToMap(self);
}
