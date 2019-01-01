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
#include "../audio/music.h"
#include "../boss/grimlore.h"
#include "../collisions.h"
#include "../entity.h"
#include "../game.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../item/key_items.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void init(void);
static void entityWait(void);
static int draw(void);
static void particleMove(void);
static void appearDone(void);
static void disappearDone(void);

Entity *addGrimloreSummonSpell(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Grimlore's Summon Spell");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &draw;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	self->startY = getMapCeiling(self->x, self->y);

	self->endY = getMapFloor(self->x, self->y) - self->h;

	if (self->targetX != -1)
	{
		playSoundToMap("sound/boss/grimlore/grimlore_summon", BOSS_CHANNEL, self->x, self->y, -1);

		self->targetX = -1;
	}

	if (self->mental == 1)
	{
		self->layer = FOREGROUND_LAYER;
	}

	setEntityAnimation(self, "APPEAR");

	self->animationCallback = &appearDone;
}

static void entityWait()
{
	if (self->active == TRUE)
	{
		if (self->health == 0)
		{
			setContinuePoint(FALSE, self->name, NULL);

			self->health = 1;
		}

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->layer = FOREGROUND_LAYER;

			if (self->mental == 0)
			{
				self->target->flags &= ~NO_DRAW;
			}

			setEntityAnimation(self, "DISAPPEAR");

			self->animationCallback = &disappearDone;
		}
	}
}

static int draw()
{
	self->y = self->startY;

	drawLoopingAnimationToMap();

	while (self->y < self->endY)
	{
		self->y += self->h;

		drawSpriteToMap();
	}

	return TRUE;
}

static void particleMove()
{
	self->y -= self->dirY;

	if (self->y < self->startY)
	{
		setEntityAnimationByID(self, prand() % 3);

		self->x = self->head->x + self->head->box.x;
		self->y = self->head->startY;

		self->startY = self->head->startY - self->h;

		self->x += prand() % (self->head->box.w - self->w);

		self->y += self->head->endY - self->head->startY + self->h;

		self->dirY = 50 + prand() % 100;

		self->dirY /= 10;
	}

	if (self->head->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}

static void appearDone()
{
	int i;
	Entity *e;

	self->action = &entityWait;

	for (i=0;i<5;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Grimlore's Summon Spell Particle");
		}

		loadProperties("boss/grimlore_summon_spell_particle", e);

		setEntityAnimationByID(e, prand() % 3);

		e->x = self->x + self->box.x;
		e->y = self->startY;

		e->startY = self->startY - e->h;

		e->x += prand() % (self->box.w - e->w);

		e->y += self->endY - self->startY + (prand() % SCREEN_HEIGHT);

		e->dirY = 50 + prand() % 100;

		e->dirY /= 10;

		e->head = self;

		e->action = &particleMove;

		e->draw = &drawLoopingAnimationToMap;
	}

	if (self->mental == 0)
	{
		e = addGrimlore(self->x, self->y, "boss/grimlore");

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		e->flags |= NO_DRAW;

		self->target = e;

		e->head = self;
	}

	else
	{
		self->target = getEntityByObjectiveName("GRIMLORE");

		self->target->flags |= NO_DRAW;

		self->layer = BACKGROUND_LAYER;
	}

	setEntityAnimation(self, "STAND");
}

static void disappearDone()
{
	Entity *temp, *e;

	if (self->mental == 0)
	{
		self->target->head = NULL;
	}

	else
	{
		temp = self;

		self = self->target;

		clearContinuePoint();

		increaseKillCount();

		freeBossHealthBar();

		e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

		e->dirY = ITEM_JUMP_HEIGHT;

		fadeBossMusic();

		entityDieVanish();

		self = temp;
	}

	stopSound(BOSS_CHANNEL);

	self->inUse = FALSE;
}
