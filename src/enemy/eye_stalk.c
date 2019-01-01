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
#include "../game.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void entityWait(void);
static void bodyWait(void);
static void createBody(void);
static void takeDamage(Entity *, int);
static void bodyTakeDamage(Entity *, int);
static void die(void);
static void riseUp(void);
static void sink(void);
static void swimAround(void);
static void alignBodyToHead(void);

Entity *addEyeStalk(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Eye Stalk");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &createBody;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->takeDamage = &takeDamage;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void riseUp()
{
	if (self->y == self->startY)
	{
		self->startY -= 128;
	}

	if (self->y > self->startY)
	{
		self->y -= 3;
	}

	else
	{
		self->y = self->startY;

		self->action = &entityWait;
	}

	alignBodyToHead();
}

static void sink()
{
	if (self->y < self->endY)
	{
		self->y += 3;
	}

	else
	{
		self->y = self->endY;

		self->dirX = self->face == RIGHT ? self->speed : -self->speed;

		self->thinkTime = 120 + prand() % 180;

		self->action = &swimAround;
	}

	alignBodyToHead();
}

static void swimAround()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->dirX = self->face == RIGHT ? -self->speed : self->speed;

		self->face = self->dirX < 0 ? LEFT : RIGHT;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &riseUp;

		self->thinkTime = 180 + prand() % 120;

		self->startX = 0;
	}

	self->endX = self->x;

	alignBodyToHead();
}

static void entityWait()
{
	self->startX++;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->x = self->endX + sin(DEG_TO_RAD(self->startX)) * 10;

	alignBodyToHead();

	if (self->maxThinkTime >= 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &sink;
		}
	}
}

static void bodyWait()
{
	checkToMap(self);
}

static void createBody()
{
	char bodyName[MAX_VALUE_LENGTH];
	int i;
	Entity **body, *head;

	self->x = self->endX;
	self->y = self->endY;

	body = malloc(self->mental * sizeof(Entity *));

	if (body == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Eye Stalk body...", self->mental * (int)sizeof(Entity *));
	}

	snprintf(bodyName, sizeof(bodyName), "%s_body", self->name);

	for (i=self->mental-1;i>=0;i--)
	{
		body[i] = getFreeEntity();

		if (body[i] == NULL)
		{
			showErrorAndExit("No free slots to add an Eye Stalk body part");
		}

		loadProperties(bodyName, body[i]);

		body[i]->x = self->x;
		body[i]->y = self->y;

		body[i]->action = &bodyWait;

		body[i]->draw = &drawLoopingAnimationToMap;
		body[i]->touch = &entityTouch;
		body[i]->die = &entityDieNoDrop;
		body[i]->takeDamage = &bodyTakeDamage;

		body[i]->type = ENEMY;

		setEntityAnimation(body[i], "STAND");
	}

	/* Recreate the head so that it's on top */

	head = getFreeEntity();

	if (head == NULL)
	{
		showErrorAndExit("No free slots to add an Eye Stalk head");
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

	self->action = self->maxThinkTime < 0 ? &riseUp : &swimAround;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		self->health -= damage;

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);
			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

			enemyPain();
		}

		else
		{
			self->damage = 0;

			if (other->type == WEAPON || other->type == PROJECTILE)
			{
				increaseKillCount();
			}

			self->die();
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}
	}
}

static void bodyTakeDamage(Entity *other, int damage)
{
	Entity *temp = self->head;

	self = self->head;

	self->takeDamage(other, damage);

	self = temp;
}

static void die()
{
	Entity *e;

	playSoundToMap("sound/enemy/tortoise/tortoise_die", -1, self->x, self->y, 0);

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

	self->die = &entityDie;

	self->die();

	self->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
	self->dirY = ITEM_JUMP_HEIGHT;
}

static void alignBodyToHead()
{
	float x, y, partDistanceX, partDistanceY;
	Entity *e;

	x = self->x;
	y = self->y;

	partDistanceX = self->endX - self->x;
	partDistanceY = fabs(self->endY - self->y);

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
