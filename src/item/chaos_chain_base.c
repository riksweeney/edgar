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

#include "../collisions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void chainBaseWait(void);
static void chainWait(void);
static void addChains(void);
static void creditsChainBaseWait(void);
static void creditsChainWait(void);
static void alignChainToChaos(void);

Entity *addChaosChainBase(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Chaos's Chain Base");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &addChains;

	e->creditsAction = &addChains;

	e->draw = &drawLoopingAnimationToMap;

	e->type = KEY_ITEM;

	setEntityAnimation(e, "STAND");

	return e;
}

static void addChains()
{
	int i;
	Entity **chains, *chaos;

	chaos = getEntityByObjectiveName("CHAOS");

	if (chaos == NULL)
	{
		showErrorAndExit("Chaos Chain Base cannot find Chaos");
	}

	chains = malloc(self->mental * sizeof(Entity *));

	if (chains == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Chaos's chains...", self->mental * (int)sizeof(Entity *));
	}

	for (i=self->mental-1;i>=0;i--)
	{
		chains[i] = getFreeEntity();

		if (chains[i] == NULL)
		{
			showErrorAndExit("No free slots to add a Chaos Chain");
		}

		loadProperties("item/chaos_chain", chains[i]);

		chains[i]->face = self->face;

		chains[i]->action = &chainWait;

		chains[i]->creditsAction = &creditsChainWait;

		chains[i]->draw = &drawLoopingAnimationToMap;

		setEntityAnimation(chains[i], "STAND");
	}

	/* Link the sections */

	for (i=self->mental-1;i>=0;i--)
	{
		if (i == 0)
		{
			self->target = chains[i];
		}

		else
		{
			chains[i - 1]->target = chains[i];
		}

		chains[i]->head = self;
	}

	free(chains);

	self->head = chaos;

	self->action = &chainBaseWait;

	self->creditsAction = &creditsChainBaseWait;
}

static void chainBaseWait()
{
	setEntityAnimation(self, self->head->animationName);

	checkToMap(self);

	alignChainToChaos();
}

static void chainWait()
{
	checkToMap(self);
}

static void creditsChainBaseWait()
{
	self->alpha = self->head->alpha;

	setEntityAnimation(self, self->head->animationName);

	checkToMap(self);

	alignChainToChaos();
}

static void creditsChainWait()
{
	self->alpha = self->head->alpha;

	checkToMap(self);
}

static void alignChainToChaos()
{
	float x, y, partDistanceX, partDistanceY;
	Entity *e;

	x = self->head->x + self->head->w - self->w - self->offsetX;

	y = self->head->y + self->offsetY;

	partDistanceX = self->x - x;
	partDistanceY = self->y - y;

	partDistanceX /= self->mental;
	partDistanceY /= self->mental;

	e = self->target;

	x += self->w / 2 - e->w / 2;
	y += self->h / 2 - e->h / 2;

	while (e != NULL)
	{
		x += partDistanceX;
		y += partDistanceY;

		e->x = x;
		e->y = y;

		if (e->target == NULL)
		{
			e->x = self->x + self->w / 2 - e->w / 2;
			e->y = self->y + self->h / 2 - e->h / 2;
		}

		e = e->target;
	}
}
