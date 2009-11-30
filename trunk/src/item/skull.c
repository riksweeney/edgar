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
#include "../collisions.h"
#include "../custom_actions.h"
#include "../graphics/graphics.h"
#include "../game.h"
#include "key_items.h"
#include "../system/error.h"

extern Entity *self;

static void wait(void);
static int draw(void);
static void activate(int);
static void chainWait(void);
static int drawChain(void);

Entity *addSkull(int x, int y, char *name)
{
	Entity *e, *chain;
	
	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Skull");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &wait;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->touch = &entityTouch;
	e->activate = &activate;
	e->die = &entityDie;

	e->draw = &draw;

	setEntityAnimation(e, STAND);
	
	chain = getFreeEntity();

	if (chain == NULL)
	{
		showErrorAndExit("No free slots to add a Skull Chain");
	}

	loadProperties("item/skull_chain", chain);
	
	chain->head = e;
	
	chain->draw = &drawChain;
	
	chain->action = &chainWait;
	
	chain->die = &entityDieVanish;

	return e;
}

static int draw()
{
	drawLoopingAnimationToMap();

	return TRUE;
}

static void chainWait()
{
	if (self->head->health > 0)
	{
		self->x = self->head->endX + self->head->w / 2 - self->w / 2;
		self->y = self->head->startY;
	}
	
	else
	{
		self->die();
	}
}

static int drawChain()
{
	int i;
	float partDistanceX, partDistanceY;

	partDistanceX = self->head->x + self->head->w / 2 - self->w / 2 - self->x;
	partDistanceY = self->head->y - self->y;

	partDistanceX /= 8;
	partDistanceY /= 8;
	
	for (i=0;i<8;i++)
	{
		drawSpriteToMap();
		
		self->x += partDistanceX;
		self->y += partDistanceY;
	}
	
	return TRUE;
}

static void wait()
{
	self->thinkTime += 2;

	if (self->thinkTime >= 360)
	{
		self->thinkTime = 0;
	}

	self->x = self->endX + sin(DEG_TO_RAD(self->thinkTime)) * 8;
}

static void activate(int val)
{
	setEntityAnimation(self, val == 1 ? WALK : STAND);
}
