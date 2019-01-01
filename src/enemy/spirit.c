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
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/graphics.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/target.h"

extern Entity *self, player;

static void hover(void);
static void touch(Entity *);
static void lookForPlayer(void);
static void teleportPlayer(void);
static void die(void);
static void moveRandomly(void);
static void moveAboveBottle(void);
static void moveIntoBottle(void);

Entity *addSpirit(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Spirit");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->touch = &touch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void lookForPlayer()
{
	float dirX;

	if (self->x <= self->startX || self->x >= self->endX)
	{
		self->x = (self->x <= self->startX ? self->startX : self->endX);

		self->dirX = 0;
	}

	if (self->dirX == 0)
	{
		self->dirX = self->face == LEFT ? self->speed : -self->speed;
	}

	self->face = self->dirX > 0 ? RIGHT : LEFT;

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->dirX = self->face == LEFT ? self->speed : -self->speed;

		self->face = self->face == LEFT ? RIGHT : LEFT;
	}

	hover();
}

static void touch(Entity *other)
{
	if (other->type == PLAYER)
	{
		teleportPlayer();

		self->action = &lookForPlayer;
	}
}

static void teleportPlayer()
{
	Target *t = getTargetByName("SPIRIT_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Spirit cannot find target");
	}

	player.targetX = t->x;
	player.targetY = t->y;

	calculatePath(player.x, player.y, player.targetX, player.targetY, &player.dirX, &player.dirY);

	player.flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	playSoundToMap("sound/common/teleport", EDGAR_CHANNEL, player.x, player.y, 0);

	activateEntitiesWithObjectiveName("SPIRIT_POINT", TRUE);
}

static void die()
{
	EntityList *list = createPixelsFromSprite(getCurrentSprite(self));
	EntityList *l;
	Entity *e;
	int i;

	i = 0;

	for (l=list->next;l!=NULL;l=l->next)
	{
		e = l->entity;

		e->target = self->target;

		e->targetX = e->x + (prand() % 160) * (prand() % 2 == 0 ? -1 : 1);
		e->targetY = e->y + (prand() % 160) * (prand() % 2 == 0 ? -1 : 1);

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->dirX *= 6;
		e->dirY *= 6;

		e->action = &moveRandomly;

		i++;
	}

	self->target->mental = i;

	freeEntityList(list);

	self->inUse = FALSE;
}

static void moveRandomly()
{
	if (fabs(self->dirX) <= 0.1 && fabs(self->dirY) <= 0.1)
	{
		self->targetX = self->target->x + (prand() % self->target->w / 2);
		self->targetY = self->target->y - 96 - (prand() % 64);

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 6;
		self->dirY *= 6;

		self->action = &moveAboveBottle;
	}

	else
	{
		self->x += self->dirX;
		self->y += self->dirY;

		self->dirX *= 0.95;
		self->dirY *= 0.95;
	}
}

static void moveAboveBottle()
{
	if (atTarget())
	{
		self->thinkTime = 60 + (prand() % 60);

		self->targetX = self->target->x + self->target->w / 2;
		self->targetY = self->target->y + self->target->h / 2;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 6;
		self->dirY *= 6;

		self->action = &moveIntoBottle;
	}

	else
	{
		self->x += self->dirX;
		self->y += self->dirY;
	}
}

static void moveIntoBottle()
{
	self->thinkTime--;

	if (atTarget())
	{
		self->target->mental--;

		self->inUse = FALSE;
	}

	else
	{
		self->x += self->dirX;
		self->y += self->dirY;
	}
}

static void hover()
{
	self->thinkTime += 5;

	if (self->thinkTime >= 360)
	{
		self->thinkTime = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->thinkTime)) * 4;
}
