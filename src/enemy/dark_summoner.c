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
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../inventory.h"
#include "../item/item.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "enemies.h"
#include "rock.h"
#include "thunder_cloud.h"

extern Entity *self, player;
extern Game game;

static void lookForPlayer(void);
static void summon(void);
static void summonWait(void);
static void hover(void);
static void summonEnd(void);
static void die(void);
static void teleportAway(void);
static void takeDamage(Entity *, int);
static void castLightningBolt(void);
static void lightningBolt(void);
static void creditsMove(void);

Entity *addDarkSummoner(int x, int y, char *name)
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

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void lookForPlayer()
{
	int rand;
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
			rand = prand() % 3;

			self->mental = 1;

			self->action = &summonWait;

			setEntityAnimation(self, "ATTACK_1");

			self->animationCallback = rand == 0 ? &castLightningBolt : &summon;

			self->dirX = 0;
		}
	}

	hover();
}

static void summonWait()
{
	checkToMap(self);

	hover();
}

static void summon()
{
	char summonList[MAX_VALUE_LENGTH], enemyToSummon[MAX_VALUE_LENGTH];
	char *token;
	int summonIndex = 0, summonCount = 0;
	Entity *e;

	STRNCPY(summonList, self->requires, MAX_VALUE_LENGTH);

	token = strtok(summonList, "|");

	while (token != NULL)
	{
		token = strtok(NULL, "|");

		summonCount++;
	}

	if (summonCount == 0)
	{
		showErrorAndExit("Summoner at %f %f has no summon list", self->x, self->y);
	}

	summonIndex = prand() % summonCount;

	STRNCPY(summonList, self->requires, MAX_VALUE_LENGTH);

	summonCount = 0;

	token = strtok(summonList, "|");

	while (token != NULL)
	{
		if (summonCount == summonIndex)
		{
			break;
		}

		token = strtok(NULL, "|");

		summonCount++;
	}

	snprintf(enemyToSummon, MAX_VALUE_LENGTH, "enemy/%s", token);

	e = addEnemy(enemyToSummon, self->x, self->y);

	e->targetX = self->x;

	e->targetY = self->y;

	e->x = e->targetX;

	e->y = e->targetY;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	self->action = &summonWait;

	self->creditsAction = &summonWait;

	setEntityAnimation(self, "ATTACK_2");

	self->animationCallback = &summonEnd;
}

static void hover()
{
	self->endX++;

	if (self->endX >= 360)
	{
		self->endX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->endX)) * 8;
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

		self->creditsAction = &creditsMove;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->thinkTime = 600;
	}

	hover();
}

static void die()
{
	Entity *e;

	playSoundToMap("sound/enemy/gazer/gazer_die", -1, self->x, self->y, 0);

	if (getInventoryItemByObjectiveName("Summoner's Staff") == NULL)
	{
		e = dropCollectableItem("item/summoner_staff", self->x + self->w / 2, self->y, self->face);

		e->x -= e->w / 2;
	}

	self->dirY = 0;

	entityDie();
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

		self->action = &summonWait;

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

static void creditsMove()
{
	if (self->health != -1)
	{
		self->targetX = self->x + SCREEN_WIDTH / 2;
		self->targetY = self->y;

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->health = -1;

		self->thinkTime = 0;
	}

	else
	{
		hover();

		self->dirX = self->speed;

		checkToMap(self);

		if (self->dirX == 0)
		{
			self->inUse = FALSE;
		}

		if (self->thinkTime == 0)
		{
			STRNCPY(self->requires, "red_centurion", MAX_VALUE_LENGTH);

			self->creditsAction = &summonWait;

			setEntityAnimation(self, "ATTACK_1");

			self->animationCallback = &summon;

			self->dirX = 0;
		}
	}
}
