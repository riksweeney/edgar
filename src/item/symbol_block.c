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
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void autoTouch(Entity *);
static void touch(Entity *);
static void entityWait(void);
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
		showErrorAndExit("No free slots to add a Symbol Block");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	e->thinkTime = 0;

	e->die = &die;

	return e;
}

static void entityWait()
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
		playSoundToMap("sound/item/block_beep", -1, self->x, self->y, 0);

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

	if (other->dirY == 0 && dirY < 0 && other->type == PLAYER)
	{
		playSoundToMap("sound/item/block_beep", -1, self->x, self->y, 0);

		self->flags &= ~FLY;

		self->dirY = -5;

		self->weight = 2;

		self->originalWeight = self->weight;

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

		self->action = self->health < 0 ? &autoSymbolChange : &entityWait;
	}
}

static void init()
{
	int required;

	if (self->active == TRUE)
	{
		self->action = self->health < 0 ? &autoSymbolChange : &entityWait;

		self->touch = self->health < 0 ? &autoTouch : &touch;
	}

	else
	{
		self->alpha = 0;

		self->action = &entityWait;

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
			showErrorAndExit("Couldn't find master block for %s", self->objectiveName);
		}

		self->thinkTime = prand() % self->maxThinkTime;

		/* Don't start on the required block otherwise the player won't adjust it */

		while (self->startY != -1 && self->thinkTime == required)
		{
			self->thinkTime = prand() % self->maxThinkTime;
		}
	}

	setEntityAnimationByID(self, self->thinkTime);
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

	setEntityAnimationByID(self, self->thinkTime);
}

static void doSymbolMatch()
{
	int i, count, remaining, required;
	Entity **list;
	EntityList *el, *entities;

	entities = getEntities();

	count = 0;

	remaining = 0;

	required = -1;

	for (el=entities->next;el!=NULL;el=el->next)
	{
		if (el->entity->inUse == TRUE && strcmpignorecase(self->objectiveName, el->entity->objectiveName) == 0)
		{
			count++;

			/* Only do validation if all the blocks have stopped */

			if (el->entity->damage != 0)
			{
				return;
			}

			if (el->entity->startY == -1)
			{
				required = el->entity->thinkTime;
			}
		}
	}

	list = malloc(count * sizeof(Entity *));

	if (list == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes for Symbol Blocks...", count * (int)sizeof(Entity *));
	}

	count = 0;

	for (el=entities->next;el!=NULL;el=el->next)
	{
		if (el->entity->inUse == TRUE && strcmpignorecase(self->objectiveName, el->entity->objectiveName) == 0)
		{
			list[count] = el->entity;

			if (el->entity->thinkTime != required)
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
			setInfoBoxMessage(60, 255, 255, 255, _("Complete"));
		}

		for (i=0;i<count;i++)
		{
			list[i]->action = &entityWait;
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
		setInfoBoxMessage(30, 255, 255, 255, _("%d more to go..."), remaining);
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

		self->action = self->health < 0 ? &autoSymbolChange : &entityWait;

		self->touch = self->health < 0 ? &autoTouch : &touch;
	}

	checkToMap(self);
}

static int getRequired()
{
	EntityList *el, *entities;

	entities = getEntities();

	for (el=entities->next;el!=NULL;el=el->next)
	{
		if (el->entity->inUse == TRUE && strcmpignorecase(self->objectiveName, el->entity->objectiveName) == 0)
		{
			if (el->entity->startY == -1)
			{
				/* Randomize the required block if not already done */

				if (el->entity->weight != 2)
				{
					el->entity->thinkTime = prand() % el->entity->maxThinkTime;

					el->entity->weight = 2;

					el->entity->originalWeight = el->entity->weight;
				}

				return el->entity->thinkTime;
			}
		}
	}

	return -1;
}
