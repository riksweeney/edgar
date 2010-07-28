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
#include "../graphics/graphics.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "../projectile.h"
#include "../map.h"
#include "../item/item.h"
#include "../game.h"
#include "../audio/music.h"
#include "../audio/audio.h"
#include "../graphics/gib.h"
#include "../item/key_items.h"
#include "../event/trigger.h"
#include "../geometry.h"
#include "../world/target.h"
#include "../hud.h"
#include "../player.h"
#include "../graphics/decoration.h"
#include "../system/error.h"

extern Entity *self, player;

static void init(void);
static void sit(void);
static void sit2(void);
static void dungeonTeleportInit(void);
static void dungeonTeleportWait(void);
static void teleportPlayerToDungeon(Entity *);
static void spellMove(void);

Entity *addSorceror(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Sorceror");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;

	e->takeDamage = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, CUSTOM_1);

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case -5:
			self->action = &sit;
		break;
		
		case -4:
			self->action = &sit2;
		break;
		
		default:
			self->action = &sit;
		break;
	}
}

static void sit()
{
	if (self->mental == -4)
	{
		setEntityAnimation(self, CUSTOM_2);
		
		playSoundToMap("sound/common/spell.ogg", BOSS_CHANNEL, self->x, self->y, 0);
		
		self->action = &dungeonTeleportInit;
	}
}

static void sit2()
{
	
}

static void dungeonTeleportInit()
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Sorceror");
	}

	loadProperties("boss/sorceror_teleport_spell", e);
	
	setEntityAnimation(e, STAND);

	e->x = self->x + e->offsetX;
	e->y = self->y + e->offsetY;

	e->action = &spellMove;
	
	calculatePath(e->x, e->y, player.x + player.w / 2, player.y + player.h / 2, &e->dirX, &e->dirY);
	
	e->dirX *= e->speed;
	e->dirY *= e->speed;

	e->draw = &drawLoopingAnimationToMap;
	
	e->touch = &teleportPlayerToDungeon;
	
	self->thinkTime = 60;
	
	self->action = &dungeonTeleportWait;
}

static void dungeonTeleportWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, CUSTOM_1);
	}
}

static void spellMove()
{
	int i;
	Entity *e;
	
	checkToMap(self);
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
	
	else
	{
		for (i=0;i<2;i++)
		{
			e = addBasicDecoration(self->x, self->y, "decoration/particle");

			if (e != NULL)
			{
				e->x += prand() % self->w;
				e->y += prand() % self->h;

				e->thinkTime = 5 + prand() % 30;

				setEntityAnimation(e, prand() % 5);
			}
		}	
	}
}

static void teleportPlayerToDungeon(Entity *other)
{
	player.flags |= NO_DRAW;
	
	self->inUse = FALSE;
	
	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	playSoundToMap("sound/common/teleport.ogg", EDGAR_CHANNEL, self->x, self->y, 0);
}
