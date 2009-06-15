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

extern Entity *self, player, entity[MAX_ENTITIES];

static void drawSuspended(void);
static void takeDamage(Entity *, int);
static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void die(void);
static void wait(void);

Entity *addFlyBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add the Fly Boss\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawSuspended;
	e->takeDamage = NULL;

	e->type = ENEMY;

	e->flags |= FLY;

	e->active = FALSE;

	setEntityAnimation(e, CUSTOM_1);

	return e;
}

static void takeDamage(Entity *other, int damage)
{
	int i;

	if (!(self->flags & INVULNERABLE))
	{
		self->health -= damage;

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);

			i = prand() % 3;

			switch (i)
			{
				case 0:
					playSound("sound/common/splat1.ogg", -1, self->x, self->y, 0);
				break;

				case 1:
					playSound("sound/common/splat2.ogg", -1, self->x, self->y, 0);
				break;

				default:
					playSound("sound/common/splat3.ogg", -1, self->x, self->y, 0);
				break;
			}
		}

		else
		{
			self->thinkTime = 180;

			self->flags &= ~FLY;

			setEntityAnimation(self, STAND);

			self->frameSpeed = 0;

			self->takeDamage = NULL;
			self->touch = NULL;

			self->action = &die;
		}
	}
}

static void initialise()
{
	int minX, minY;

	minX = getMapStartX();
	minY = getMapStartY();
	
	if (self->active == TRUE)
	{
		if (cameraAtMinimum())
		{
			centerMapOnEntity(NULL);
			
			self->dirX = self->speed;

			setEntityAnimation(self, CUSTOM_1);

			self->action = &doIntro;

			self->flags &= ~NO_DRAW;
			self->flags &= ~FLY;

			self->thinkTime = 120;
			
			printf("Starting\n");
		}
	}
}

static void doIntro()
{
	int i;
	Entity *e;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->flags |= NO_DRAW;
		
		for (i=0;i<11;i++)
		{
			e = addTemporaryItem("boss/fly_boss_cocoon_piece", self->x, self->y, RIGHT, 0, 0);
	
			e->x += (self->w - e->w) / 2;
			e->y += (self->w - e->w) / 2;
	
			e->dirX = (prand() % 3) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);
	
			setEntityAnimation(e, i);
	
			e->thinkTime = 180 + (prand() % 60);
		}
		
		playSound("sound/boss/fly_boss/buzz.ogg", BOSS_CHANNEL, self->x, self->y, 0);
		
		self->takeDamage = &takeDamage;
		
		self->action = &introPause;
		
		self->thinkTime = 120;
	}
}

static void introPause()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		playBossMusic();
		
		self->action = &wait;
	}
}

static void wait()
{

}

static void drawSuspended()
{
	drawLine(self->startX + self->w / 2, self->startY, self->startX + self->w / 2, self->y, 255, 255, 255);
	
	drawLoopingAnimationToMap();
}

static void die()
{

}
