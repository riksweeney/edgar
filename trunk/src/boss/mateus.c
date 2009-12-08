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
#include "../system/properties.h"
#include "../entity.h"
#include "../enemy/rock.h"
#include "../collisions.h"
#include "../geometry.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../audio/music.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../graphics/decoration.h"
#include "../game.h"
#include "../hud.h"
#include "../map.h"
#include "../player.h"
#include "../system/error.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void wait(void);
static void hover(void);
static void raiseBlocks(void);
static void fireBlockTouch(Entity *);
static void fireBlockEnd(void);
static void blockFire(void);
static void fireBlockWait(void);
static void raiseBlocksWait(void);
static void attackFinished(void);
static void fireBlockWait(void);
static void knifeThrowInit(void);
static void knifeWait(void);
static void knifeAttack(void);
static void knifeBlock(void);

Entity *addMateus(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Mateus");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->die = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	self->thinkTime = 180;

	setEntityAnimation(self, CUSTOM_1);

	self->action = &doIntro;
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, CUSTOM_2);

		self->animationCallback = &introPause;
	}
}

static void introPause()
{
	hover();

	self->dirX = 0.5;

	checkToMap(self);

	self->thinkTime--;

	playBossMusic();

	initBossHealthBar();

	self->action = &attackFinished;

	self->thinkTime = 90;

	facePlayer();
}

static void attackFinished()
{
	self->frameSpeed = 1;

	self->dirX = 0;

	self->thinkTime = 90;

	self->damage = 1;

	self->action = &wait;

	self->touch = &entityTouch;

	hover();
}

static void wait()
{
	int i;
	
	self->dirX = 0;

	facePlayer();

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		i = prand() % 2;
		
		switch (i)
		{
			case 0:
				self->mental = 0;
		
				self->action = &raiseBlocks;
			break;
			
			default:
				self->action = &knifeThrowInit;
			break;
		}
	}

	hover();
}

static void raiseBlocksWait()
{
	if (self->mental <= 0)
	{
		self->action = &attackFinished;
	}

	hover();
}

static void hover()
{
	self->startX++;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 8;
}

static void raiseBlocks()
{
	int i, x, y;
	Entity *e;

	x = getMapStartX();
	y = getMapFloor(self->x, self->y);

	for (i=0;i<19;i++)
	{
		e = getFreeEntity();

		e->x = x;
		e->y = y;

		e->action = &initialise;

		e->draw = &drawLoopingAnimationToMap;
		e->touch = fireBlockTouch;
		e->die = NULL;
		e->takeDamage = NULL;
		e->action = &fireBlockWait;

		e->head = self;

		e->targetY = e->y - 240;

		e->maxThinkTime = 180;

		e->thinkTime = 180 + (i * 60);

		e->type = ENEMY;

		setEntityAnimation(e, STAND);

		x += TILE_SIZE;

		self->mental++;
	}

	self->action = &raiseBlocksWait;
}

static void fireBlockWait()
{
	if (self->y > self->targetY)
	{
		self->y -= 4;
	}

	else
	{
		self->y = self->targetY;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->thinkTime = 30;

			self->health = 1;

			self->action = &blockFire;

			self->thinkTime = self->maxThinkTime;
		}
	}
}

static void blockFire()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental--;

		self->health = 0;

		if (self->mental <= 0)
		{
			self->head->mental--;

			self->action = &fireBlockEnd;
		}

		else
		{
			self->thinkTime = self->maxThinkTime;

			self->action = &fireBlockWait;
		}
	}
}

static void fireBlockTouch(Entity *other)
{
	if (self->health == 1)
	{
		entityTouch(other);
	}

	pushEntity(other);
}

static void fireBlockEnd()
{
	Entity *e;

	if (self->head->mental <= 0)
	{
		playSoundToMap("sound/common/crumble.ogg", BOSS_CHANNEL, self->x, self->y, 0);

		e = addSmallRock(self->x, self->y, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		e->dirX = -3;
		e->dirY = -8;

		e = addSmallRock(self->x, self->y, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		e->dirX = 3;
		e->dirY = -8;

		self->inUse = FALSE;
	}
}

static void knifeThrowInit()
{
	int i, radians;
	Entity *e;

	for (i=0;i<8;i++)
	{
		e = getFreeEntity();

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		radians = DEG_TO_RAD(i * 45);

		e->x += (0 * cos(radians) - 0 * sin(radians));
		e->y += (0 * sin(radians) + 0 * cos(radians));

		e->action = &knifeWait;
		e->touch = &entityTouch;
		e->draw = &drawLoopingAnimationToMap;

		e->health = radians;

		e->head = self;

		e->thinkTime = 120;
	}
}

static void knifeWait()
{
	float startX, startY, endX, endY;
	
	self->mental += 4;

	if (self->mental >= 128)
	{
		self->mental = 128;
	}

	self->x = self->head->x + self->head->w / 2 - self->w / 2;
	self->y = self->head->y + self->head->h / 2 - self->h / 2;

	self->x += (self->mental * cos(self->health) - 0 * sin(self->health));
	self->y += (self->mental * sin(self->health) + 0 * cos(self->health));

	if (self->head->mental == 1)
	{
		self->action = &knifeAttack;

		self->reactToBlock = &knifeBlock;

		startX = self->x;
		startY = self->y;

		endX = player.x + player.w / 2;
		endY = player.y + player.h / 2;

		calculatePath(startX, startY, endX, endY, &self->dirX, &self->dirY);

		self->dirX *= 12;
		self->dirY *= 12;

		self->thinkTime = 60;
	}
}

static void knifeAttack()
{
	float dirX, dirY;

	dirX = self->dirX;
	dirY = self->dirY;

	checkToMap(self);

	if ((self->dirX == 0 && dirX != 0) || (self->dirY == 0 && dirY != 0))
	{
		self->action = &entityDieNoDrop;
	}
}

static void knifeBlock()
{
	self->flags &= ~FLY;

	self->dirX = self->x < player.x ? -5 : 5;

	self->dirY = -5;

	self->thinkTime = 120;
}
