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
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../map.h"
#include "../game.h"
#include "../audio/music.h"
#include "../collisions.h"
#include "../item/key_items.h"
#include "../world/target.h"
#include "../graphics/decoration.h"
#include "../player.h"
#include "../geometry.h"
#include "../hud.h"
#include "../game.h"
#include "../enemy/enemies.h"
#include "../projectile.h"
#include "../system/error.h"

extern Entity *self;

static void init(void);
static void wait(void);
static void takeDamage(Entity *, int);

Entity *addAwesomeBossMeter(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Awesome Boss Meter");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	
	e->takeDamage = &takeDamage;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	if (self->active == TRUE)
	{
		initBossHealthBar();
		
		self->action = &wait;
		
		self->thinkTime = 180;
		
		self->health = 0;
	}
}

static void wait()
{
	Entity *e;
	
	if (self->damage == 0)
	{
		self->thinkTime--;
		
		self->flags &= ~FLY;
		
		if (self->thinkTime <= 0)
		{
			clearContinuePoint();
			
			freeBossHealthBar();

			e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);
			
			e->y -= e->h;

			e->dirY = ITEM_JUMP_HEIGHT;

			fadeBossMusic();
			
			entityDieVanish();
		}
	}
	
	else if (self->damage == self->mental && self->health == self->maxHealth)
	{
		self->targetY--;
		
		if (self->targetY <= 0)
		{
			self->targetY = 0;
		}
	}
	
	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{
	if (self->health < self->maxHealth)
	{
		self->health += damage;
		
		if (self->health >= self->maxHealth)
		{
			self->health = self->maxHealth;
			
			setInfoBoxMessage(120, 255, 255, 255, _("Super is ready..."));
			
			self->targetX = prand() % 2;
			
			self->targetY = 120;
		}
	}
}
