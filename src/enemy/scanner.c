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
#include "../enemy/enemies.h"
#include "../entity.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

static void init(void);
static void lookForPlayer(void);
static void followPlayer(void);
static void closedEyeMove(void);
static void summonEnemies(void);
static void teleportPlayer(void);
static void creditsMove(void);
static void blueCreditsMove(void);

extern Entity *self, player;

Entity *addScanner(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Scanner");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;

	e->creditsAction = strcmpignorecase("enemy/blue_scanner", e->name) == 0 ? &blueCreditsMove : &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	self->endY = getMapFloor(self->x, self->y) - self->y;

	self->action = strcmpignorecase("enemy/blue_scanner", self->name) == 0 ? &closedEyeMove : &lookForPlayer;
}

static void lookForPlayer()
{
	int frame;
	float timer;

	if (self->active == TRUE)
	{
		self->flags &= ~NO_DRAW;

		moveLeftToRight();

		if (self->currentFrame == 3)
		{
			if (self->health == 0)
			{
				playSoundToMap("sound/enemy/gazer/flap", -1, self->x, self->y, 0);

				self->health = 1;
			}
		}

		else
		{
			self->health = 0;
		}

		if (player.health > 0 && player.alpha == 255 && collision(self->x + self->w / 2 - 10, self->y, 20, self->endY, player.x, player.y, player.w, player.h) == 1)
		{
			playSoundToMap("sound/enemy/gazer/growl", -1, self->x, self->y, 0);

			setInfoBoxMessage(120, 255, 255, 255, _("INTRUDER!"));

			self->thinkTime = 300;

			activateEntitiesWithRequiredName(self->objectiveName, FALSE);

			if (self->mental == 1)
			{
				summonEnemies();
			}

			frame = self->currentFrame;
			timer = self->frameTimer;

			setEntityAnimation(self, "ATTACK_1");

			self->currentFrame = frame;
			self->frameTimer = timer;

			self->target = &player;

			self->action = &followPlayer;
		}
	}

	else
	{
		self->flags |= NO_DRAW;
	}
}

static void closedEyeMove()
{
	int frame;
	float timer;

	moveLeftToRight();

	if (self->currentFrame == 3)
	{
		if (self->health == 0)
		{
			playSoundToMap("sound/enemy/gazer/flap", -1, self->x, self->y, 0);

			self->health = 1;
		}
	}

	else
	{
		self->health = 0;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental = 1 - self->mental;

		frame = self->currentFrame;
		timer = self->frameTimer;

		setEntityAnimation(self, self->mental == 0 ? "STAND" : "WALK");

		self->currentFrame = frame;
		self->frameTimer = timer;

		self->thinkTime = self->maxThinkTime;
	}

	if (player.health > 0 && self->mental == 0 && collision(self->x + self->w / 2 - 10, self->y, 20, self->endY, player.x, player.y, player.w, player.h) == 1)
	{
		playSoundToMap("sound/enemy/gazer/growl", -1, self->x, self->y, 0);

		setInfoBoxMessage(120, 255, 255, 255, _("INTRUDER!"));

		self->thinkTime = 300;

		activateEntitiesWithRequiredName(self->objectiveName, FALSE);

		if (self->damage == 1)
		{
			summonEnemies();
		}

		frame = self->currentFrame;
		timer = self->frameTimer;

		setEntityAnimation(self, "ATTACK_1");

		self->currentFrame = frame;
		self->frameTimer = timer;

		self->target = &player;

		if (self->damage == 2)
		{
			self->thinkTime = 120;

			self->action = &teleportPlayer;
		}

		else
		{
			self->action = &followPlayer;
		}
	}
}

static void teleportPlayer()
{
	int frame;
	float timer, x, y;

	if (self->target != NULL)
	{
		getCheckpoint(&x, &y);

		self->target->targetX = x;
		self->target->targetY = y;

		calculatePath(self->target->x, self->target->y, self->target->targetX, self->target->targetY, &self->target->dirX, &self->target->dirY);

		self->target->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		playSoundToMap("sound/common/teleport", (self->target->type == PLAYER ? EDGAR_CHANNEL : -1), self->target->x, self->target->y, 0);

		self->target = NULL;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		frame = self->currentFrame;
		timer = self->frameTimer;

		setEntityAnimation(self, "STAND");

		self->currentFrame = frame;
		self->frameTimer = timer;

		self->action = strcmpignorecase("enemy/blue_scanner", self->name) == 0 ? &closedEyeMove : &lookForPlayer;
	}
}

static void followPlayer()
{
	int frame;
	float timer;

	self->targetX = self->target->x - self->w / 2 + self->target->w / 2;

	if (self->speed != 0)
	{
		/* Position under the player */

		if (abs(self->x - self->targetX) <= self->speed * 3)
		{
			self->dirX = 0;
		}

		else
		{
			self->dirX = self->targetX < self->x ? -self->target->speed * 3 : self->target->speed * 3;
		}
	}

	checkToMap(self);

	if (player.health <= 0 || collision(self->x, self->y, self->w, self->endY, player.x, player.y, player.w, player.h) == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			activateEntitiesWithRequiredName(self->objectiveName, TRUE);

			frame = self->currentFrame;
			timer = self->frameTimer;

			setEntityAnimation(self, "STAND");

			self->currentFrame = frame;
			self->frameTimer = timer;

			self->action = strcmpignorecase("enemy/blue_scanner", self->name) == 0 ? &closedEyeMove : &lookForPlayer;
		}
	}
}

static void summonEnemies()
{
	char summonList[MAX_VALUE_LENGTH], enemyToSummon[MAX_VALUE_LENGTH];
	char *token;
	int i, summonIndex = 0, summonCount = 0;
	Entity *e;

	for (i=0;i<2;i++)
	{
		summonCount = 0;

		summonIndex = 0;

		STRNCPY(summonList, self->requires, MAX_VALUE_LENGTH);

		token = strtok(summonList, "|");

		while (token != NULL)
		{
			token = strtok(NULL, "|");

			summonCount++;
		}

		if (summonCount == 0)
		{
			showErrorAndExit("Scanner at %f %f has no summon list", self->x, self->y);
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

		e->x = self->x;

		e->y = self->y;

		e->targetX = self->x + (i == 0 ? -64 : 64);

		e->targetY = self->y;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING);
	}
}

static void creditsMove()
{
	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}

static void blueCreditsMove()
{
	setEntityAnimation(self, "ATTACK_1");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}
