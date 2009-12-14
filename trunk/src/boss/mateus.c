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
#include "../item/key_items.h"
#include "../item/item.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../world/weak_wall.h"

extern Entity *self, player;
extern Game game;

static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void wait(void);
static void hover(void);
static void raiseBlocks(void);
static void lightingBlockTouch(Entity *);
static void lightingBlockEnd(void);
static void lightingBlockAttack(void);
static void lightingBlockWait(void);
static void raiseLightningBlocksWait(void);
static void attackFinished(void);
static void lightingBlockWait(void);
static void knifeThrowInit(void);
static void knifeWait(void);
static void knifeAttack(void);
static void knifeBlock(void);
static void knifeBlockWait(void);
static void takeDamage(Entity *, int);
static void knifeThrow(void);

Entity *addMataeus(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Mataeus");
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
	
	self->startY = self->y;
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
	self->dirX = 0.5;

	checkToMap(self);

	playBossMusic();

	initBossHealthBar();

	self->action = &attackFinished;

	self->thinkTime = 90;

	facePlayer();
}

static void attackFinished()
{
	self->startY = self->y;
	
	self->frameSpeed = 1;

	self->dirX = 0;

	self->thinkTime = 90;

	self->damage = 1;

	self->action = &wait;

	self->touch = &entityTouch;

	self->takeDamage = &takeDamage;
	
	setEntityAnimation(self, STAND);

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
		
		self->mental = 0;

		self->action = &knifeThrowInit;
	}

	hover();
}

static void raiseLightningBlocksWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->mental = 0;
		
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

	for (i=0;i<20;i++)
	{
		e = addWeakWall("wall/mataeus_wall", x, y);

		e->action = &initialise;

		e->draw = &drawLoopingAnimationToMap;
		e->touch = &lightingBlockTouch;
		e->die = NULL;
		e->takeDamage = NULL;
		e->action = &lightingBlockWait;

		e->head = self;

		e->targetY = e->y - 180;

		e->type = ENEMY;
		
		e->dirY = -1;

		setEntityAnimation(e, STAND);

		x += TILE_SIZE;
		
		e->maxThinkTime = i;
	}
	
	self->thinkTime = 600;
	
	self->mental = 1;

	self->action = &raiseLightningBlocksWait;
}

static void lightingBlockWait()
{
	if (self->y <= self->targetY)
	{
		self->dirY = 0;
		
		self->y = self->targetY;

		if (self->mental == 1)
		{
			self->thinkTime = 30;
			
			self->action = &lightingBlockAttack;
			
			setEntityAnimation(self, ATTACK_1);
			
			self->damage = 1;
		}
		
		if (self->head->mental <= 0)
		{
			self->thinkTime = self->maxThinkTime * 15;
			
			self->action = &lightingBlockEnd;
		}
	}
	
	checkToMap(self);
}

static void lightingBlockAttack()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->damage = 0;
		
		self->mental = 0;
		
		setEntityAnimation(self, STAND);
	}
}

static void lightingBlockTouch(Entity *other)
{
	entityTouch(other);

	pushEntity(other);
	
	if (self->mental == 0 && other->standingOn == self)
	{
		self->mental = 1;
	}
}

static void lightingBlockEnd()
{
	Entity *e;

	self->thinkTime--;
	
	if (self->thinkTime <= 0)
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

	for (i=0;i<6;i++)
	{
		e = getFreeEntity();
		
		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Mataeus Knife");
		}
		
		radians = prand() % 7;
		
		if (radians == 0)
		{
			loadProperties("boss/mataeus_knife_special", e);
			
			e->reactToBlock = &knifeBlock;
		}
		
		else
		{
			loadProperties("boss/mataeus_knife", e);
			
			e->reactToBlock = &bounceOffShield;
		}

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		radians = DEG_TO_RAD(i * 60);

		e->x += (0 * cos(radians) - 0 * sin(radians));
		e->y += (0 * sin(radians) + 0 * cos(radians));

		e->action = &knifeWait;
		e->touch = &entityTouch;
		e->draw = &drawLoopingAnimationToMap;

		e->health = radians;

		e->head = self;
		
		setEntityAnimation(e, WALK);
	}
	
	self->thinkTime = 30;
	
	self->action = &knifeThrow;
}

static void knifeThrow()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		if (self->mental == 0)
		{
			self->mental = 1;
		}
		
		else
		{
			self->action = &attackFinished;
		}
		
		self->thinkTime = 120;
	}
	
	hover();
}

static void knifeWait()
{
	float startX, startY, endX, endY;

	self->mental += 4;

	if (self->mental >= 64)
	{
		self->mental = 64;
	}

	self->x = self->head->x + self->head->w / 2 - self->w / 2;
	self->y = self->head->y + self->head->h / 2 - self->h / 2;

	self->x += (self->mental * cos(self->health) - 0 * sin(self->health));
	self->y += (self->mental * sin(self->health) + 0 * cos(self->health));

	if (self->head->mental == 1)
	{
		self->action = &knifeAttack;

		startX = self->x;
		startY = self->y;

		endX = player.x + player.w / 2;
		endY = player.y;

		calculatePath(startX, startY, endX, endY, &self->dirX, &self->dirY);

		self->dirX *= 30;
		self->dirY *= 30;

		self->thinkTime = 60;
		
		facePlayer();
		
		setEntityAnimation(self, ATTACK_1);
		
		self->health = 1;
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

	self->damage = 0;

	self->action = &knifeBlockWait;
	
	self->activate = &throwItem;
}

static void knifeBlockWait()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->thinkTime = 120;
		
		self->dirX = 0;
		
		self->action = &generalItemAction;

		self->touch = &keyItemTouch;

		self->activate = &throwItem;
	}
}

static void takeDamage(Entity *other, int damage)
{
	if (self->health > 1000)
	{
		if (strcmpignorecase(other->name, "item/mataeus_knife") != 0)
		{
			playSoundToMap("sound/common/dink.ogg", EDGAR_CHANNEL, self->x, self->y, 0);

			if (prand() % 10 == 0)
			{
				setInfoBoxMessage(60, _("This weapon is not having any effect..."));
			}

			setCustomAction(self, &invulnerableNoFlash, 20, 0);
		}

		else
		{
			self->health -= damage;

			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);

			other->inUse = FALSE;

			if (self->health <= 1000)
			{
				self->takeDamage = &entityTakeDamageNoFlinch;
			}
		}
	}
}
