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
#include "../custom_actions.h"
#include "../entity.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../item/item.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/target.h"
#include "book.h"
#include "rock.h"
#include "thunder_cloud.h"

extern Entity *self, player;

static void hover(void);
static void redWait(void);
static void redDie(void);
static void redDieInit(void);
static void shudder(void);
static void castFireInit(void);
static void throwFire(void);
static void castFire(void);
static void castFireFinish(void);
static void fireDrop(void);
static void fireFall(void);
static void fireMove(void);
static void fireBounce(void);
static void fireBlock(Entity *);
static void fireAttack(void);
static void fireAttackPause(void);
static void blueWait(void);
static void blueDieInit(void);
static void blueDie(void);
static void castIce(void);
static void castIceFinish(void);
static void iceDrop(void);
static void iceAttack(void);
static void castIceInit(void);
static void iceAttackPause(void);
static void createIceWall(void);
static void iceWallMove(void);
static void iceWallTouch(Entity *);
static void teleportToOtherSide(void);
static void createIceBlock(void);
static void iceBlockDrop(void);
static void iceBlockTakeDamage(Entity *, int);
static void iceBlockTouch(Entity *);
static void iceBlockDie(void);
static void yellowWait(void);
static void yellowDieInit(void);
static void yellowDie(void);
static void castLightningInit(void);
static void createThunderCloud(void);
static void createThunderCloudFinish(void);
static void lightningAttack(void);
static void createLightningWave(void);
static void createLightningWaveInit(void);
static void lightningAttackPause(void);
static void castLightningFinish(void);
static void lightningWaveStart(void);
static void castLightningBolt(void);
static void lightningBolt(void);
static void greenWait(void);
static void greenDieInit(void);
static void greenDie(void);
static void physicalAttack(void);
static void physicalAttackPause(void);
static void physicalAttackInit(void);
static void physicalAttackFinish(void);
static void summon(void);
static void summon2(void);
static void followPlayer(void);
static void dropOnPlayer(void);
static void dropWait(void);
static void takeDamage(Entity *, int);
static void creditsMove(void);
static void greenCreditsMove(void);
static void creditsSummon(void);

Entity *addLargeBook(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Book");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->takeDamage = &takeDamage;

	if (strcmpignorecase(name, "enemy/large_red_book") == 0)
	{
		e->action = &redWait;
		e->die = &redDieInit;

		e->creditsAction = &creditsMove;
	}

	else if (strcmpignorecase(name, "enemy/large_blue_book") == 0)
	{
		e->action = &blueWait;
		e->die = &blueDieInit;

		e->creditsAction = &creditsMove;
	}

	else if (strcmpignorecase(name, "enemy/large_yellow_book") == 0)
	{
		e->action = &yellowWait;
		e->die = &yellowDieInit;

		e->creditsAction = &creditsMove;
	}

	else
	{
		e->action = &greenWait;
		e->die = &greenDieInit;

		e->creditsAction = &greenCreditsMove;
	}

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void redWait()
{
	if (self->active == TRUE)
	{
		self->action = &fireAttack;
	}

	checkToMap(self);

	hover();
}

static void redDieInit()
{
	Target *t;

	self->touch = NULL;

	self->maxThinkTime++;

	switch (self->maxThinkTime)
	{
		case 2:
			t = getTargetByName("RED_BOOK_TARGET_1");

			activateEntitiesWithRequiredName("RED_BOOK_STAGE_1", TRUE);

			self->action = &redDie;
		break;

		default:
			t = getTargetByName("RED_BOOK_TARGET_2");

			activateEntitiesWithRequiredName("RED_BOOK_STAGE_2", TRUE);

			self->action = &redDie;
		break;
	}

	if (t == NULL)
	{
		showErrorAndExit("Red Book cannot find target");
	}

	self->startX = self->x;
	self->startY = 0;

	self->targetX = t->x;
	self->targetY = t->y;

	self->thinkTime = 60;
}

static void redDie()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->maxThinkTime == 4)
		{
			increaseKillCount();

			entityDie();
		}

		else
		{
			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

			playSoundToMap("sound/common/teleport", BOSS_CHANNEL, self->x, self->y, 0);

			self->touch = &entityTouch;

			self->action = &redWait;

			self->active = FALSE;

			self->startX = self->targetX;
			self->startY = self->targetY;

			self->health = self->maxHealth;
		}
	}

	else
	{
		shudder();
	}
}

static void fireAttack()
{
	self->thinkTime--;

	if (player.health > 0 && self->thinkTime <= 0)
	{
		self->endY = self->endY == 1 ? 0 : 1;

		self->action = &castFireInit;
	}

	facePlayer();

	checkToMap(self);

	hover();
}

static void fireAttackPause()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &fireAttack;

		switch (self->maxThinkTime)
		{
			case 1:
				self->mental = 5;
			break;

			case 2:
				self->mental = 3;
			break;

			default:
				self->mental = 3;
			break;
		}
	}

	checkToMap(self);

	hover();
}

static void castFireInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->endX = 7;

		self->action = self->maxThinkTime == 3 ? &throwFire : &castFire;

		playSoundToMap("sound/enemy/fireball/fireball", -1, self->x, self->y, 0);
	}

	hover();
}

static void throwFire()
{
	int i;
	float dir;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		for (i=0;i<5;i++)
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

			dir = self->mental == 2 ? 1.25 : 1.75;

			e->dirY = -9;
			e->dirX = self->face == LEFT ? -(i * dir) : (i * dir);
			e->weight = 0.4;
			e->originalWeight = e->weight;

			e->thinkTime = 20;

			e->action = &fireFall;
			e->draw = &drawLoopingAnimationToMap;
			e->touch = &entityTouch;
			e->reactToBlock = &fireBlock;

			e->head = self;

			e->face = self->face;

			e->type = ENEMY;

			e->flags |= DO_NOT_PERSIST;

			setEntityAnimation(e, "DOWN");
		}
	}

	self->thinkTime = 60;

	self->action = &castFireFinish;
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

		e->head = self;

		e->face = self->face;

		e->type = ENEMY;

		e->flags |= DO_NOT_PERSIST;

		setEntityAnimation(e, "DOWN");

		self->endX--;

		if (self->endX <= 0)
		{
			self->thinkTime = 30;

			self->action = &castFireFinish;
		}

		else
		{
			self->thinkTime = 3;
		}
	}
}

static void castFireFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
        self->mental--;

		if (self->mental <= 0)
		{
			self->thinkTime = 180;

			self->action = self->maxThinkTime == 2 ? &teleportToOtherSide : &fireAttackPause;
		}

		else
		{
			self->action = &fireAttack;
		}
	}
}

static void fireDrop()
{
	if (self->flags & ON_GROUND)
	{
		switch (self->head->maxThinkTime)
		{
			case 1:
				self->dirX = (self->face == LEFT ? -self->speed : self->speed);

				self->action = &fireMove;
			break;

			case 2:
				self->dirX = (self->face == LEFT ? -4 : 4);

				self->action = &fireBounce;
			break;

			default:
				self->inUse = FALSE;
			break;
		}
	}

	checkToMap(self);
}

static void fireFall()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->inUse = FALSE;
		}
	}
}

static void fireMove()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}

static void fireBounce()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirY = -15;
	}

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}

static void fireBlock(Entity *other)
{
	self->dirX = 0;
}

static void blueWait()
{
	if (self->active == TRUE)
	{
		self->action = &iceAttack;
	}

	checkToMap(self);

	hover();
}

static void blueDieInit()
{
	Target *t;

	self->touch = NULL;

	self->maxThinkTime++;

	switch (self->maxThinkTime)
	{
		case 2:
			t = getTargetByName("BLUE_BOOK_TARGET_1");

			activateEntitiesWithRequiredName("BLUE_BOOK_STAGE_1", TRUE);

			self->action = &blueDie;
		break;

		default:
			t = getTargetByName("BLUE_BOOK_TARGET_2");

			activateEntitiesWithRequiredName("BLUE_BOOK_STAGE_2", TRUE);

			self->action = &blueDie;
		break;
	}

	if (t == NULL)
	{
		showErrorAndExit("Blue Book cannot find target");
	}

	self->startX = self->x;
	self->startY = 0;

	self->targetX = t->x;
	self->targetY = t->y;

	self->thinkTime = 60;
}

static void blueDie()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->maxThinkTime == 4)
		{
			increaseKillCount();

			entityDie();
		}

		else
		{
			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

			playSoundToMap("sound/common/teleport", BOSS_CHANNEL, self->x, self->y, 0);

			self->touch = &entityTouch;

			self->action = &blueWait;

			self->active = FALSE;

			self->startX = self->targetX;
			self->startY = self->targetY;

			self->health = self->maxHealth;
		}
	}

	else
	{
		shudder();
	}
}

static void iceAttack()
{
	self->thinkTime--;

	if (player.health > 0 && self->thinkTime <= 0)
	{
		self->endY = self->endY == 1 ? 0 : 1;

		self->action = &castIceInit;
	}

	facePlayer();

	checkToMap(self);

	hover();
}

static void castIceInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->maxThinkTime == 1)
		{
			self->endX = 5 + prand() % 6;

			self->action = &castIce;
		}

		else if (self->maxThinkTime == 2)
		{
			self->endX = 3 + prand() % 4;

			self->action = &createIceWall;
		}

		else
		{
			self->endX = 5 + prand() % 6;

			self->action = &createIceBlock;
		}
	}

	hover();
}

static void castIce()
{
	int x, y;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Ice Spike");
		}

		loadProperties("enemy/ice_spike", e);

		setEntityAnimation(e, "STAND");

		x = player.x + player.w / 2;
		y = self->y - 100 - (prand() % 60);

		e->x = x;
		e->y = y;

		if (isValidOnMap(e) == TRUE)
		{
			e->x = self->x + self->w / 2;
			e->y = self->y + self->h / 2;

			e->x -= e->w / 2;
			e->y -= e->h / 2;

			e->targetX = x;
			e->targetY = y;

			calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

			e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

			playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

			e->action = &iceDrop;
			e->draw = &drawLoopingAnimationToMap;
			e->touch = &entityTouch;

			e->head = self;

			e->face = self->face;

			e->type = ENEMY;

			e->thinkTime = 30;

			e->flags |= FLY|DO_NOT_PERSIST;

			self->endX--;

			if (self->endX <= 0)
			{
				self->thinkTime = 0;

				self->action = &castIceFinish;
			}

			else
			{
				self->thinkTime = 30;
			}
		}

		else
		{
			e->inUse = FALSE;
		}
	}

	checkToMap(self);

	hover();
}

static void createIceWall()
{
	int i, mapFloor;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		mapFloor = getMapFloor(self->x, self->y);

		for (i=0;i<2;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add Ice Wall");
			}

			loadProperties("enemy/ice_wall", e);

			e->x = player.x + player.w / 2 + (i == 0 ? -200 : 200);
			e->y = mapFloor + 50;

			e->face = (i == 0 ? RIGHT : LEFT);

			e->targetY = mapFloor - e->h;

			e->x -= e->w / 2;

			e->action = &iceWallMove;
			e->draw = &drawLoopingAnimationToMap;
			e->touch = &iceWallTouch;

			e->head = self;

			e->type = ENEMY;

			e->flags |= DO_NOT_PERSIST;

			e->head = self;

			e->thinkTime = 60;

			setEntityAnimation(e, "STAND");
		}

		self->endX--;

		if (self->endX <= 0)
		{
			self->thinkTime = 0;

			self->action = &teleportToOtherSide;
		}

		else
		{
			self->thinkTime = 90;
		}
	}

	checkToMap(self);

	hover();
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

static void iceDrop()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~FLY;
	}

	checkToMap(self);

	if ((self->flags & ON_GROUND) || self->standingOn != NULL)
	{
		playSoundToMap("sound/common/shatter", -1, self->x, self->y, 0);

		for (i=0;i<8;i++)
		{
			e = addTemporaryItem("misc/ice_spike_piece", self->x, self->y, RIGHT, 0, 0);

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
}

static void iceAttackPause()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &iceAttack;
	}

	checkToMap(self);

	hover();
}

static void castIceFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 180;

		self->action = &iceAttackPause;
	}
}

static void createIceBlock()
{
	int x, y;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Ice Block");
		}

		loadProperties("edgar/edgar_frozen", e);

		setEntityAnimation(e, "STAND");

		x = player.x + player.w / 2;
		y = self->y - 100 - (prand() % 60);

		e->x = x;
		e->y = y;

		if (isValidOnMap(e) == TRUE)
		{
			e->x = self->x + self->w / 2;
			e->y = self->y + self->h / 2;

			e->x -= e->w / 2;
			e->y -= e->h / 2;

			e->targetX = x;
			e->targetY = y;

			e->damage = 1;

			e->health = 50;

			calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

			e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

			playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

			e->action = &iceBlockDrop;
			e->draw = &drawLoopingAnimationToMap;
			e->touch = &entityTouch;
			e->pain = &enemyPain;
			e->die = &iceBlockDie;

			e->head = self;

			e->face = self->face;

			e->type = ENEMY;

			e->thinkTime = 30;

			e->flags |= FLY|DO_NOT_PERSIST;

			self->endX--;

			if (self->endX <= 0)
			{
				self->thinkTime = 0;

				self->action = &castIceFinish;
			}

			else
			{
				self->thinkTime = 60;
			}
		}

		else
		{
			e->inUse = FALSE;
		}
	}

	checkToMap(self);

	hover();
}

static void iceBlockDrop()
{
	if (self->standingOn != NULL)
	{
		if (strcmpignorecase(self->name, self->standingOn->name) == 0)
		{
			self->standingOn->action = self->standingOn->die;
		}

		else
		{
			self->die();
		}
	}

	self->thinkTime--;

	if (self->thinkTime <= 0 && (self->flags & FLY))
	{
		self->flags &= ~FLY;

		self->thinkTime = 300;
	}

	if (self->flags & ON_GROUND)
	{
		self->touch = &iceBlockTouch;

		self->takeDamage = &iceBlockTakeDamage;

		if (self->thinkTime <= 0)
		{
			self->die();
		}
	}

	checkToMap(self);
}

static void iceBlockTouch(Entity *other)
{
	Entity *temp;

	pushEntity(other);

	if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}
	}

	else if (other->type == PROJECTILE && other->parent != self)
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}

		temp = self;

		self = other;

		self->die();

		self = temp;
	}
}

void iceBlockTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (strcmpignorecase(other->name, "weapon/pickaxe") == 0)
	{
		self->damage = 0;

		self->die();
	}

	else if (damage != 0)
	{
		self->health -= damage;

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

			if (self->pain != NULL)
			{
				self->pain();
			}
		}

		else
		{
			self->damage = 0;

			self->die();
		}
	}
}

static void iceBlockDie()
{
	int i;
	Entity *e;

	for (i=0;i<8;i++)
	{
		e = addTemporaryItem("common/ice_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += self->w / 2 - e->w / 2;
		e->y += self->h / 2 - e->h / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->thinkTime = 60 + (prand() % 60);

		e->touch = NULL;
	}

	playSoundToMap("sound/common/shatter", -1, player.x, player.y, 0);

	self->inUse = FALSE;
}

static void yellowWait()
{
	if (self->active == TRUE)
	{
		self->action = &lightningAttack;
	}

	checkToMap(self);

	hover();
}

static void yellowDieInit()
{
	Target *t;

	self->touch = NULL;

	self->maxThinkTime++;

	switch (self->maxThinkTime)
	{
		case 2:
			t = getTargetByName("YELLOW_BOOK_TARGET_1");

			activateEntitiesWithRequiredName("YELLOW_BOOK_STAGE_1", TRUE);

			self->action = &yellowDie;
		break;

		default:
			t = getTargetByName("YELLOW_BOOK_TARGET_2");

			activateEntitiesWithRequiredName("YELLOW_BOOK_STAGE_2", TRUE);

			self->action = &yellowDie;
		break;
	}

	if (t == NULL)
	{
		showErrorAndExit("Yellow Book cannot find target");
	}

	self->startX = self->x;
	self->startY = 0;

	self->targetX = t->x;
	self->targetY = t->y;

	self->thinkTime = 60;
}

static void yellowDie()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->maxThinkTime == 4)
		{
			increaseKillCount();

			entityDie();
		}

		else
		{
			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

			playSoundToMap("sound/common/teleport", BOSS_CHANNEL, self->x, self->y, 0);

			self->touch = &entityTouch;

			self->action = &yellowWait;

			self->active = FALSE;

			self->startX = self->targetX;
			self->startY = self->targetY;

			self->health = self->maxHealth;
		}
	}

	else
	{
		shudder();
	}
}

static void lightningAttack()
{
	self->thinkTime--;

	if (player.health > 0 && self->thinkTime <= 0)
	{
		self->endY = self->endY == 1 ? 0 : 1;

		self->action = &castLightningInit;
	}

	facePlayer();

	checkToMap(self);

	hover();
}

static void castLightningInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->maxThinkTime == 1)
		{
			self->action = &createThunderCloud;
		}

		else if (self->maxThinkTime == 2)
		{
			facePlayer();

			self->endX = 0;

			self->targetX = self->x + (self->face == RIGHT ? 64 : -64);

			self->action = &createLightningWaveInit;
		}

		else
		{
			facePlayer();

			self->endX = 5;

			self->targetX = self->x + (self->face == RIGHT ? 64 : -64);

			self->action = &castLightningBolt;
		}
	}

	hover();
}

static void createThunderCloud()
{
	Entity *e = addThunderCloud(self->x, self->y, "enemy/thunder_cloud");

	e->x = self->x + self->w / 2;
	e->y = self->y + self->h / 2;

	e->x -= e->w / 2;
	e->y -= e->h / 2;

	e->targetX = self->x + self->w / 2;
	e->targetY = self->y - 150;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

	playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

	e->head = self;

	e->face = RIGHT;

	self->action = &createThunderCloudFinish;
}

static void createThunderCloudFinish()
{
	facePlayer();

	checkToMap(self);

	hover();
}

static void createLightningWaveInit()
{
	Entity *e;

	if (self->endX == 0)
	{
		e = addThunderCloud(self->x, self->y, "enemy/thunder_cloud");

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		e->targetX = self->targetX;
		e->targetY = getMapCeiling(self->x, self->y);

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

		e->head = self;

		e->face = RIGHT;

		setEntityAnimation(e, "STAND");

		e->action = &lightningWaveStart;

		self->endX = 1;
	}

	checkToMap(self);

	hover();
}

static void lightningWaveStart()
{
	self->head->action = &createLightningWave;

	self->inUse = FALSE;
}

static void createLightningWave()
{
	int i, top, bottom, valid;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		bottom = getMapFloor(self->x, self->y);
		top    = getMapCeiling(self->x, self->y);

		valid = TRUE;

		playSoundToMap("sound/enemy/thunder_cloud/lightning", -1, self->x, self->y, 0);

		for (i=top;i<bottom;i+=32)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add lightning");
			}

			loadProperties("enemy/lightning", e);

			setEntityAnimation(e, "STAND");

			e->x = self->targetX + self->w / 2 - e->w / 2;
			e->y = i;

			e->action = &lightningWait;

			e->draw = &drawLoopingAnimationToMap;
			e->touch = &entityTouch;

			e->head = self;

			e->currentFrame = prand() % 6;

			e->face = RIGHT;

			e->thinkTime = 15;

			if (isValidOnMap(e) == FALSE)
			{
				valid = FALSE;

				e->inUse = FALSE;

				break;
			}
		}

		if (valid == TRUE)
		{
			e = addSmallRock(self->targetX, bottom, "common/small_rock");

			e->x += (self->w - e->w) / 2;
			e->y -= e->h;

			e->dirX = -3;
			e->dirY = -8;

			e = addSmallRock(self->targetX, bottom, "common/small_rock");

			e->x += (self->w - e->w) / 2;
			e->y -= e->h;

			e->dirX = 3;
			e->dirY = -8;

			self->targetX += self->face == RIGHT ? 64 : -64;

			self->thinkTime = 30;
		}

		else
		{
			self->action = &teleportToOtherSide;
		}
	}

	checkToMap(self);

	hover();
}

static void lightningAttackPause()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &lightningAttack;
	}

	checkToMap(self);

	hover();
}

static void castLightningFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 180;

		self->action = &lightningAttackPause;
	}
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
		e->targetY = getMapCeiling(self->x, self->y);

		e->startY = e->targetY;
		e->endY   = getMapFloor(e->targetX, e->targetY);

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

		e->head = self;

		e->face = RIGHT;

		e->action = &lightningBolt;

		e->draw = &drawLoopingAnimationToMap;

		e->head = self;

		e->face = self->face;

		e->type = ENEMY;

		e->thinkTime = 0;

		e->flags |= FLY|DO_NOT_PERSIST;

		self->endX--;

		if (self->endX <= 0)
		{
			self->thinkTime = 0;

			self->action = &castLightningFinish;
		}

		else
		{
			self->thinkTime = 30;
		}
	}

	checkToMap(self);

	hover();
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

static void shudder()
{
	self->startY += 90;

	self->x = self->startX + sin(DEG_TO_RAD(self->startY)) * 4;

	if (self->startY >= 360)
	{
		self->startY = 0;
	}
}

static void hover()
{
	self->startX += 5;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 4;
}

static void teleportToOtherSide()
{
	Target *t;

	if (strcmpignorecase(self->name, "enemy/large_red_book") == 0)
	{
		t = getTargetByName("RED_BOOK_LEFT_SIDE");

		if (t == NULL)
		{
			showErrorAndExit("Red Book cannot find target");
		}

		if ((int)t->x == (int)self->x)
		{
			t = getTargetByName("RED_BOOK_TARGET_1");

			if (t == NULL)
			{
				showErrorAndExit("Red Book cannot find target");
			}
		}

		self->action = &fireAttackPause;
	}

	else if (strcmpignorecase(self->name, "enemy/large_blue_book") == 0)
	{
		t = getTargetByName("BLUE_BOOK_RIGHT_SIDE");

		if (t == NULL)
		{
			showErrorAndExit("Blue Book cannot find target");
		}

		if ((int)t->x == (int)self->x)
		{
			t = getTargetByName("BLUE_BOOK_TARGET_1");

			if (t == NULL)
			{
				showErrorAndExit("Blue Book cannot find target");
			}
		}

		self->action = &castIceFinish;
	}

	else if (strcmpignorecase(self->name, "enemy/large_yellow_book") == 0)
	{
		t = getTargetByName("YELLOW_BOOK_RIGHT_SIDE");

		if (t == NULL)
		{
			showErrorAndExit("Yellow Book cannot find target");
		}

		if ((int)t->x == (int)self->x)
		{
			t = getTargetByName("YELLOW_BOOK_TARGET_1");

			if (t == NULL)
			{
				showErrorAndExit("Yellow Book cannot find target");
			}
		}

		self->action = &castLightningFinish;
	}

	else
	{
		t = getTargetByName("GREEN_BOOK_RIGHT_SIDE");

		if (t == NULL)
		{
			showErrorAndExit("Green Book cannot find target");
		}

		if ((int)t->x == (int)self->x)
		{
			t = getTargetByName("GREEN_BOOK_TARGET_2");

			if (t == NULL)
			{
				showErrorAndExit("Green Book cannot find target");
			}
		}

		self->action = &physicalAttackPause;
	}

	self->targetX = t->x;
	self->targetY = t->y;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	playSoundToMap("sound/common/teleport", BOSS_CHANNEL, self->x, self->y, 0);
}

static void greenWait()
{
	if (self->active == TRUE)
	{
		self->action = &physicalAttack;
	}

	checkToMap(self);

	hover();
}

static void greenDieInit()
{
	Target *t;

	self->touch = NULL;

	self->maxThinkTime++;

	switch (self->maxThinkTime)
	{
		case 2:
			t = getTargetByName("GREEN_BOOK_TARGET_1");

			activateEntitiesWithRequiredName("GREEN_BOOK_STAGE_1", TRUE);

			self->action = &greenDie;
		break;

		default:
			t = getTargetByName("GREEN_BOOK_TARGET_2");

			activateEntitiesWithRequiredName("GREEN_BOOK_STAGE_2", TRUE);

			self->action = &greenDie;
		break;
	}

	if (t == NULL)
	{
		showErrorAndExit("Green Book cannot find target");
	}

	self->startX = self->x;
	self->startY = 0;

	self->targetX = t->x;
	self->targetY = t->y;

	self->thinkTime = 60;
}

static void greenDie()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->maxThinkTime == 4)
		{
			increaseKillCount();

			entityDie();
		}

		else
		{
			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

			playSoundToMap("sound/common/teleport", BOSS_CHANNEL, self->x, self->y, 0);

			self->touch = &entityTouch;

			self->action = &greenWait;

			self->active = FALSE;

			self->startX = self->targetX;
			self->startY = self->targetY;

			self->health = self->maxHealth;
		}
	}

	else
	{
		shudder();
	}
}

static void physicalAttack()
{
	self->thinkTime--;

	if (player.health > 0 && self->thinkTime <= 0)
	{
		self->endY = self->endY == 1 ? 0 : 1;

		self->action = &physicalAttackInit;
	}

	facePlayer();

	checkToMap(self);

	hover();
}

static void physicalAttackInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->maxThinkTime == 1)
		{
			self->endX = 3;

			self->face = RIGHT;

			self->action = &summon;
		}

		else if (self->maxThinkTime == 2)
		{
			self->endX = 3;

			self->mental = 0;

			self->action = &followPlayer;
		}

		else if (self->maxThinkTime == 3)
		{
			self->endX = 1;

			self->action = &summon2;
		}
	}

	hover();
}

static void summon()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addBook(self->x, self->y, "enemy/green_book");

		e->pain = &enemyPain;

		e->targetX = player.x + player.w / 2 - e->w / 2 + 32;
		e->targetY = player.y - 32 - prand() % 64;

		e->startY = e->targetY;

		e->x = e->targetX;
		e->y = e->targetY;

		if (isValidOnMap(e) == TRUE)
		{
			e->x = self->x + self->w / 2;
			e->y = self->y + self->h / 2;

			e->x -= e->w / 2;
			e->y -= e->h / 2;

			e->health = 30;

			calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

			e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

			playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

			e->face = RIGHT;

			self->endX--;
		}

		else
		{
			e->inUse = FALSE;
		}

		if (self->endX <= 0)
		{
			self->thinkTime = 300;

			self->action = &physicalAttackFinish;
		}

		else
		{
			self->thinkTime = 45;
		}
	}

	checkToMap(self);

	hover();
}

static void followPlayer()
{
	self->targetX = player.x - self->w / 2 + player.w / 2;

	/* Position above the player */

	if (getDistanceFromPlayer(self) > SCREEN_WIDTH)
	{
		self->dirX = 0;

		hover();
	}

	else if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;

		self->thinkTime = 15;

		if (player.health > 0)
		{
			self->action = &dropOnPlayer;
		}

		else
		{
			hover();
		}
	}

	else
	{
		self->dirX = self->targetX < self->x ? -player.speed * 3 : player.speed * 3;
	}

	checkToMap(self);
}

static void dropOnPlayer()
{
	int i;
	long onGround;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~FLY;

		onGround = (self->flags & ON_GROUND);

		checkToMap(self);

		if (onGround == 0 && (self->flags & ON_GROUND))
		{
			playSoundToMap("sound/common/crash", BOSS_CHANNEL, self->x, self->y, 0);

			if ((player.flags & ON_GROUND) && !(player.flags & INVULNERABLE))
			{
				setPlayerStunned(150);
			}

			shakeScreen(MEDIUM, 15);

			self->thinkTime = 15;

			self->action = &dropWait;

			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}

			self->endX--;

			if (self->endX == 0)
			{
				self->mental = 1;

				self->endX = 4;
			}

			self->targetY = self->y;
		}
	}
}

static void dropWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= FLY;

		if (self->y < self->startY)
		{
			self->y = self->startY;

			self->dirY = 0;

			self->action = &followPlayer;
		}

		else
		{
			if (self->mental == 1 && self->y < self->targetY - 64)
			{
				self->dirY = 0;

				self->mental = 0;

				self->action = &dropOnPlayer;
			}

			self->dirY = -4;
		}
	}

	checkToMap(self);
}

static void summon2()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		i = prand() % 4;

		switch (i)
		{
			case 0:
				e = addBook(self->x, self->y, "enemy/green_book");
			break;

			case 1:
				e = addBook(self->x, self->y, "enemy/yellow_book");
			break;

			case 2:
				e = addBook(self->x, self->y, "enemy/red_book");
			break;

			default:
				e = addBook(self->x, self->y, "enemy/blue_book");
			break;
		}

		e->pain = &enemyPain;

		e->targetX = player.x + player.w / 2 - e->w / 2 + (self->face == RIGHT ? 64 : -64);
		e->targetY = player.y - 32 - prand() % 64;

		e->startY = e->targetY;

		e->x = e->targetX;
		e->y = e->targetY;

		if (isValidOnMap(e) == TRUE)
		{
			e->x = self->x + self->w / 2;
			e->y = self->y + self->h / 2;

			e->x -= e->w / 2;
			e->y -= e->h / 2;

			e->health = 30;

			calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

			e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

			playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

			e->face = RIGHT;

			self->endX--;
		}

		else
		{
			e->inUse = FALSE;
		}

		if (self->endX <= 0)
		{
			self->thinkTime = 120;

			self->action = &physicalAttackFinish;
		}

		else
		{
			self->thinkTime = 45;
		}
	}

	facePlayer();

	checkToMap(self);

	hover();
}

static void physicalAttackPause()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &physicalAttack;
	}

	checkToMap(self);

	hover();
}

static void physicalAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 180;

		self->action = self->maxThinkTime == 3 ? &teleportToOtherSide : &physicalAttackPause;
	}

	checkToMap(self);

	hover();
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if ((other->element == LIGHTNING && strcmpignorecase(self->name, "enemy/large_yellow_book") == 0)
		|| (other->element == FIRE && strcmpignorecase(self->name, "enemy/large_red_book") == 0))
	{
		if (damage != 0)
		{
			self->health += damage;

			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

			if (self->pain != NULL)
			{
				self->pain();
			}

			if (prand() % 5 == 0)
			{
				setInfoBoxMessage(90, 255, 255, 255, _("The damage from this weapon is being absorbed..."));
			}
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}
	}

	else
	{
		if (damage != 0)
		{
			self->health -= damage;

			if (other->type == PROJECTILE)
			{
				temp = self;

				self = other;

				self->die();

				self = temp;
			}

			if (self->health > 0)
			{
				setCustomAction(self, &flashWhite, 6, 0, 0);

				/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

				if (other->type != PROJECTILE)
				{
					setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
				}

				if (self->pain != NULL)
				{
					self->pain();
				}
			}

			else
			{
				self->die();
			}
		}
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

	hover();
}

static void greenCreditsMove()
{
	self->thinkTime++;

	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}

	hover();

	if (self->thinkTime == 300)
	{
		self->thinkTime = 60;

		self->creditsAction = &creditsSummon;

		self->mental = 0;
	}
}

static void creditsSummon()
{
	Entity *e;

	self->dirX = 0;

	hover();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		switch (self->mental)
		{
			case 0:
				e = addBook(self->x, self->y, "enemy/green_book");
			break;

			case 1:
				e = addBook(self->x, self->y, "enemy/yellow_book");
			break;

			case 2:
				e = addBook(self->x, self->y, "enemy/red_book");
			break;

			default:
				e = addBook(self->x, self->y, "enemy/blue_book");

				self->creditsAction = &creditsMove;
			break;
		}

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		e->targetY = e->y + (prand() % 16) * (prand() % 2 == 0 ? -1 : 1);
		e->targetX = e->x - SCREEN_WIDTH / 2 + (prand() % 16) * (prand() % 2 == 0 ? -1 : 1);

		e->startY = e->targetY;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

		e->face = RIGHT;

		self->mental++;

		self->thinkTime = 30;
	}
}
