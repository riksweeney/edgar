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
#include "../system/properties.h"
#include "../entity.h"
#include "../medal.h"
#include "../enemy/enemies.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../map.h"
#include "../audio/music.h"
#include "../event/trigger.h"
#include "../item/key_items.h"
#include "../item/item.h"
#include "../collisions.h"
#include "../event/script.h"
#include "../custom_actions.h"
#include "../hud.h"
#include "../game.h"
#include "../inventory.h"
#include "../player.h"
#include "../graphics/graphics.h"
#include "../geometry.h"
#include "../graphics/decoration.h"
#include "../world/target.h"
#include "../enemy/rock.h"
#include "../item/grimlore_artifact.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../enemy/magic_missile.h"
#include "../boss/grimlore.h"
#include "../world/explosion.h"

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
	
	setEntityAnimation(self, "APPEAR");
	
	self->animationCallback = &appearDone;
}

static void entityWait()
{
	if (self->active == TRUE)
	{
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			self->layer = FOREGROUND_LAYER;
			
			self->target->flags &= ~NO_DRAW;
			
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
	
	e = addGrimlore(self->x, self->y, "boss/grimlore");
	
	e->x = self->x + self->w / 2 - e->w / 2;
	e->y = self->y + self->h / 2 - e->h / 2;
	
	e->flags |= NO_DRAW;
	
	self->target = e;
	
	e->head = self;
	
	setEntityAnimation(self, "STAND");
}

static void disappearDone()
{
	self->target->head = NULL;
	
	self->inUse = FALSE;
}
