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
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "key_items.h"
#include "item.h"
#include "../hud.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "../system/random.h"

extern Entity *self, entity[MAX_ENTITIES];

static void autoTouch(Entity *);
static void touch(Entity *);
static void wait(void);
static void init(void);
static void autoSymbolChange(void);
static void changeSymbol(int);
static void doSymbolMatch(void);
static void die(void);
static void bounce(void);
static void appear(void);
static int getRequired();

Entity *addSymbolBlock(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Symbol Block\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->thinkTime = 0;

	e->die = &die;

	return e;
}

static void wait()
{
	if (self->touch == NULL && self->active == TRUE)
	{
		self->action = &appear;
	}

	checkToMap(self);
}

static void autoSymbolChange()
{
	if (self->startY == -1)
	{
		self->damage = 0;
	}

	else if (self->damage > 0)
	{
		self->damage--;

		if (self->damage == 0)
		{
			changeSymbol(1);

			self->damage = 30;
		}
	}
}

static void autoTouch(Entity *other)
{
	float dirY;

	dirY = other->dirY;

	pushEntity(other);

	if (other->dirY == 0 && dirY < 0)
	{
		self->damage = self->damage == 0 ? 1 : 0;

		self->flags &= ~FLY;

		self->dirY = -5;

		self->action = &bounce;

		if (self->damage == 0)
		{
			doSymbolMatch();
		}
	}
}

static void touch(Entity *other)
{
	float dirY;

	dirY = other->dirY;

	pushEntity(other);

	if (other->dirY == 0 && dirY < 0)
	{
		self->flags &= ~FLY;

		self->dirY = -5;

		self->weight = 2;

		self->action = &bounce;

		changeSymbol(1);

		doSymbolMatch();
	}
}

static void bounce()
{
	checkToMap(self);

	if (self->y >= self->endY)
	{
		self->y = self->endY;

		self->flags |= FLY;

		self->dirY = 0;

		self->action = self->health < 0 ? &autoSymbolChange : &wait;
	}
}

static void init()
{
	int required;

	if (self->active == TRUE)
	{
		self->action = self->health < 0 ? &autoSymbolChange : &wait;

		self->touch = self->health < 0 ? &autoTouch : &touch;
	}

	else
	{
		self->alpha = 0;

		self->action = &wait;

		self->touch = NULL;
	}

	if (self->health >= 0)
	{
		self->damage = 0;
	}

	/* Don't re-randomise if the player has adjusted it */

	if (self->weight != 2)
	{
		required = getRequired();

		if (required == -1)
		{
			printf("Couldn't find master block for %s\n", self->objectiveName);

			exit(0);
		}

		self->thinkTime = prand() % self->maxThinkTime;

		/* Don't start on the required block otherwise the player won't adjust it */

		while (self->startY != -1 && self->thinkTime == required)
		{
			self->thinkTime = prand() % self->maxThinkTime;
		}

		/* Only randomize the required block once since it's not reachable */

		if (self->startY == -1)
		{
			self->weight = 2;
		}
	}

	setEntityAnimation(self, self->thinkTime);
}

static void changeSymbol(int val)
{
	self->thinkTime += val;

	if (self->thinkTime >= self->maxThinkTime)
	{
		self->thinkTime = 0;
	}

	else if (self->thinkTime < 0)
	{
		self->thinkTime = self->maxThinkTime - 1;
	}

	setEntityAnimation(self, self->thinkTime);
}

static void doSymbolMatch()
{
	int i, count, remaining, required;
	Entity **list;

	count = 0;

	remaining = 0;

	required = -1;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && strcmpignorecase(self->objectiveName, entity[i].objectiveName) == 0)
		{
			count++;

			/* Only do validation if all the blocks have stopped */

			if (entity[i].damage != 0)
			{
				return;
			}

			if (entity[i].startY == -1)
			{
				required = entity[i].thinkTime;
			}
		}
	}

	list = (Entity **)malloc(count * sizeof(Entity *));

	if (list == NULL)
	{
		printf("Failed to allocate %d bytes for Symbol Blocks...\n", count * (int)sizeof(Entity *));
	}

	count = 0;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && strcmpignorecase(self->objectiveName, entity[i].objectiveName) == 0)
		{
			list[count] = &entity[i];

			if (entity[i].thinkTime != required)
			{
				remaining++;
			}

			count++;
		}
	}

	if (remaining == 0)
	{
		if (self->health >= 0)
		{
			setInfoBoxMessage(30,  _("Complete"));
		}

		for (i=0;i<count;i++)
		{
			list[i]->action = &wait;
			list[i]->touch = NULL;

			list[i]->thinkTime = 60;

			list[i]->action = &die;
		}

		activateEntitiesWithRequiredName(self->objectiveName, FALSE);
	}

	else if (self->health < 0)
	{
		for (i=0;i<count;i++)
		{
			list[i]->damage = 30 + prand() % 30;
		}
	}

	else if (self->thinkTime == required)
	{
		setInfoBoxMessage(30,  _("%d more to go..."), remaining);
	}

	free(list);
}

static void die()
{
	self->flags |= DO_NOT_PERSIST;

	self->alpha -= 3;

	if (self->alpha <= 0)
	{
		self->inUse = FALSE;
	}
}

static void appear()
{
	self->alpha += 3;

	if (self->alpha >= 255)
	{
		self->alpha = 255;

		self->action = self->health < 0 ? &autoSymbolChange : &wait;

		self->touch = self->health < 0 ? &autoTouch : &touch;
	}

	checkToMap(self);
}

static int getRequired()
{
	int i;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].inUse == TRUE && strcmpignorecase(self->objectiveName, entity[i].objectiveName) == 0)
		{
			if (entity[i].startY == -1)
			{
				return entity[i].thinkTime;
			}
		}
	}

	return -1;
}
