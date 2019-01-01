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
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../item/item.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "rock.h"
#include "thunder_cloud.h"

extern Entity *self, player;
extern Game game;

static void lookForPlayer(void);
static void hover(void);
static void summonEnd(void);
static void die(void);
static void teleportAway(void);
static void takeDamage(Entity *, int);
static void castLightningBolt(void);
static void castFireInit(void);
static void castFire(void);
static void fireDrop(void);
static void fireMove(void);
static void fireBlock(Entity *);
static void castIce(void);
static void iceWallMove(void);
static void iceWallTouch(Entity *);
static void lightningBolt(void);
static void castWait(void);
static void ballWait(void);
static void moveToSorceror(void);
static void flashWait(void);

Entity *addSorcerorDarkSummoner(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Dark Summoner");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->takeDamage = &takeDamage;
	e->reactToBlock = &changeDirection;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void lookForPlayer()
{
	float dirX;

	self->thinkTime--;

	if (self->dirX == 0)
	{
		self->dirX = self->face == LEFT ? self->speed : -self->speed;
	}

	self->face = self->dirX > 0 ? RIGHT : LEFT;

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->dirX = self->face == LEFT ? self->speed : -self->speed;

		self->face = self->face == LEFT ? RIGHT : LEFT;
	}

	if (self->thinkTime <= 0 && player.health > 0 && prand() % 30 == 0)
	{
		self->thinkTime = 0;

		if (collision(self->x + (self->face == RIGHT ? self->w : -160), self->y, 160, 200, player.x, player.y, player.w, player.h) == 1)
		{
			self->mental = 1;

			self->action = &castWait;

			setEntityAnimation(self, "ATTACK_1");

			switch (self->maxThinkTime)
			{
				case 2:
					self->animationCallback = &castIce;
				break;

				case 1:
					self->animationCallback = &castFireInit;
				break;

				default:
					self->animationCallback = &castLightningBolt;
				break;
			}

			self->dirX = 0;
		}
	}

	hover();
}

static void castWait()
{
	checkToMap(self);

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

static void summonEnd()
{
	setEntityAnimation(self, "STAND");

	if (self->thinkTime > 0)
	{
		self->thinkTime--;
	}

	else
	{
		if (prand() % 3 == 0)
		{
			self->action = &teleportAway;
		}

		else
		{
			self->action = &lookForPlayer;

			self->mental = 0;
		}

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->thinkTime = 600;
	}

	hover();
}

static void die()
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Energy Drainer Ball");
	}

	loadProperties("enemy/energy_drainer_ball", e);

	setEntityAnimation(e, "STAND");

	e->target = self->head;

	e->x = self->x + self->w / 2;
	e->y = self->y + self->h / 2;

	e->targetX = e->target->x + e->target->w / 2 - e->w / 2;
	e->targetY = e->target->y + e->target->h / 2 - e->h / 2;

	e->x -= e->w / 2;
	e->y -= e->h / 2;

	e->startY = e->y;

	e->action = &ballWait;
	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->thinkTime = 90;

	playSoundToMap("sound/enemy/gazer/gazer_die", -1, self->x, self->y, 0);

	entityDie();
}

static void ballWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed;
		self->dirY *= self->speed;

		self->action = &moveToSorceror;
	}

	else
	{
		self->endY += 5;

		self->y = self->startY + cos(DEG_TO_RAD(self->endY)) * 16;
	}
}

static void moveToSorceror()
{
	Entity *e;

	self->x += self->dirX;
	self->y += self->dirY;

	if (fabs(self->x - self->targetX) <= fabs(self->dirX) && fabs(self->y - self->targetY) <= fabs(self->dirY))
	{
		self->target->startY++;

		self->inUse = FALSE;

		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Sorceror Energy Flash");
		}

		loadProperties("boss/sorceror_energy_flash", e);

		setEntityAnimation(e, "STAND");

		e->head = self->target;

		e->face = self->target->face;

		e->x = e->head->x;
		e->y = e->head->y;

		e->action = &flashWait;
		e->draw = &drawLoopingAnimationToMap;
	}
}

static void flashWait()
{
	self->x = self->head->x;
	self->y = self->head->y;

	self->alpha -= 12;

	if (self->alpha <= 0)
	{
		self->inUse = FALSE;
	}
}

static void takeDamage(Entity *other, int damage)
{
	entityTakeDamageNoFlinch(other, damage);

	if (self->mental == 0 && self->health > 0 && (prand() % 3 == 0))
	{
		self->action = &teleportAway;
	}
}

static void teleportAway()
{
	float x;

	x = self->x;

	self->targetX = self->x < player.x ? player.x + 128 : player.x - 128 - self->w;

	self->targetY = self->y;

	self->x = self->targetX;

	if (isValidOnMap(self) == TRUE)
	{
		self->x = x;

		self->face = self->x < player.x ? RIGHT : LEFT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->animationCallback = NULL;

		setEntityAnimation(self, "STAND");

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		playSoundToMap("sound/common/teleport", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &lookForPlayer;

		/* Don't reset thinkTime if teleporting after summon */

		if (self->mental == 0)
		{
			self->thinkTime = 0;
		}
	}

	else
	{
		self->x = x;

		if (self->mental == 0)
		{
			self->action = &lookForPlayer;
		}
	}

	self->mental = 0;
}

static void castFireInit()
{
	self->endX = 5;

	playSoundToMap("sound/enemy/fireball/fireball", -1, self->x, self->y, 0);

	self->action = &castFire;
}

static void castFire()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Fire");
		}

		loadProperties("enemy/fire", e);

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		e->action = &fireDrop;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;
		e->reactToBlock = &fireBlock;

		e->face = self->face;

		e->type = ENEMY;

		e->thinkTime = 600;

		e->flags |= DO_NOT_PERSIST;

		setEntityAnimation(e, "DOWN");

		self->endX--;

		if (self->endX <= 0)
		{
			self->action = &castWait;

			setEntityAnimation(self, "ATTACK_2");

			self->animationCallback = &summonEnd;
		}

		else
		{
			self->thinkTime = 3;
		}
	}
}

static void fireDrop()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}

	else if (self->flags & ON_GROUND)
	{
		self->thinkTime = 600;

		self->dirX = (self->face == LEFT ? -self->speed : self->speed);

		self->action = &fireMove;
	}

	checkToMap(self);
}

static void fireMove()
{
	checkToMap(self);

	self->thinkTime--;

	if (self->dirX == 0 || self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}

static void fireBlock(Entity *other)
{
	self->dirX = 0;
}

static void castLightningBolt()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add lightning");
		}

		loadProperties("enemy/lightning", e);

		setEntityAnimation(e, "STAND");

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		e->targetX = player.x + player.w / 2 - e->w / 2;
		e->targetY = getMapCeiling(e->targetX, self->y);

		e->startY = e->targetY;
		e->endY   = getMapFloor(e->targetX, e->targetY);

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		e->head = self;

		e->face = RIGHT;

		e->action = &lightningBolt;

		e->draw = &drawLoopingAnimationToMap;

		e->head = self;

		e->face = self->face;

		e->type = ENEMY;

		e->thinkTime = 0;

		e->flags |= FLY|DO_NOT_PERSIST;

		self->action = &castWait;

		setEntityAnimation(self, "ATTACK_2");

		self->animationCallback = &summonEnd;
	}
}

static void lightningBolt()
{
	int i, middle;
	Entity *e;

	self->flags |= NO_DRAW;

	self->thinkTime--;

	middle = -1;

	if (self->thinkTime <= 0)
	{
		playSoundToMap("sound/enemy/thunder_cloud/lightning", -1, self->targetX, self->startY, 0);

		for (i=self->startY;i<self->endY;i+=32)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add lightning");
			}

			loadProperties("enemy/lightning", e);

			setEntityAnimation(e, "STAND");

			if (i == self->startY)
			{
				middle = self->targetX + self->w / 2 - e->w / 2;
			}

			e->x = middle;
			e->y = i;

			e->action = &lightningWait;

			e->draw = &drawLoopingAnimationToMap;
			e->touch = &entityTouch;

			e->head = self;

			e->currentFrame = prand() % 6;

			e->face = RIGHT;

			e->thinkTime = 15;
		}

		e = addSmallRock(self->x, self->endY, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y -= e->h;

		e->dirX = -3;
		e->dirY = -8;

		e = addSmallRock(self->x, self->endY, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y -= e->h;

		e->dirX = 3;
		e->dirY = -8;

		self->inUse = FALSE;
	}
}

static void castIce()
{
	int i, mapFloor;
	Entity *e;

	mapFloor = getMapFloor(self->head->x + self->head->w / 2, self->head->y);

	for (i=0;i<2;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Ice Wall");
		}

		loadProperties("enemy/ice_wall", e);

		e->x = player.x + player.w / 2 + (i == 0 ? -200 : 200);
		e->y = mapFloor;

		e->face = (i == 0 ? RIGHT : LEFT);

		e->targetY = mapFloor - e->h;

		e->x -= e->w / 2;

		e->action = &iceWallMove;
		e->draw = &drawLoopingAnimationToMap;

		e->head = self;

		e->type = ENEMY;

		e->flags |= DO_NOT_PERSIST;

		e->head = self;

		e->thinkTime = 60;

		setEntityAnimation(e, "STAND");
	}

	self->action = &castWait;

	setEntityAnimation(self, "ATTACK_2");

	self->animationCallback = &summonEnd;
}

static void iceWallMove()
{
	int i;
	Entity *e;

	if (self->y > self->targetY)
	{
		self->y -= 12;

		if (self->y <= self->targetY)
		{
			self->y = self->targetY;

			playSoundToMap("sound/common/crumble", BOSS_CHANNEL, self->x, self->y, 0);

			shakeScreen(MEDIUM, 15);

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

			self->touch = &iceWallTouch;
		}
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			self->dirX = self->face == LEFT ? -self->speed : self->speed;
		}

		else if (self->thinkTime < 0 && self->dirX == 0)
		{
			playSoundToMap("sound/common/shatter", -1, self->x, self->y, 0);

			for (i=0;i<8;i++)
			{
				e = addTemporaryItem("misc/ice_wall_piece", self->x, self->y, RIGHT, 0, 0);

				e->x = self->x + self->w / 2;
				e->x -= e->w / 2;

				e->y = self->y + self->h / 2;
				e->y -= e->h / 2;

				e->dirX = (prand() % 4) * (prand() % 2 == 0 ? -1 : 1);
				e->dirY = ITEM_JUMP_HEIGHT * 2 + (prand() % ITEM_JUMP_HEIGHT);

				setEntityAnimationByID(e, i);

				e->thinkTime = 60 + (prand() % 60);

				e->touch = NULL;
			}

			self->inUse = FALSE;
		}

		checkToMap(self);
	}
}

static void iceWallTouch(Entity *other)
{
	Entity *temp;

	if (other->type == PLAYER && self->parent != other && self->damage != 0)
	{
		temp = self;

		self = other;

		self->takeDamage(temp, temp->damage);

		self = temp;
	}

	else if (other->head == self->head)
	{
		other->dirX = 0;

		self->dirX = 0;
	}
}
