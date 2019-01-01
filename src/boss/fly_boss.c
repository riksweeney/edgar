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
#include "../audio/music.h"
#include "../collisions.h"
#include "../credits.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/graphics.h"
#include "../hud.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../map.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/target.h"

extern Entity *self, player;

static int drawSuspended(void);
static void takeDamage(Entity *, int);
static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void entityWait(void);
static void hover(void);
static void moveToTarget(void);
static void flyToTopTarget(void);
static void bulletFireInit(void);
static void bulletFireMoveToPosition(void);
static void fireBullets(void);
static void slimeFireInit(void);
static void slimeFireMoveToPosition(void);
static void fireSlime(void);
static void attackFinished(void);
static void slimePlayer(Entity *);
static void headButtInit(void);
static void headButtMoveToPosition(void);
static void moveToHeadButtRange(void);
static void headButt(void);
static void headButtFinish(void);
static void selectRandomBottomTarget(void);
static void reactToHeadButtBlock(Entity *);
static void dropInit(void);
static void drop(void);
static void dropWait(void);
static void stingAttackInit(void);
static void stingAttackMoveToPosition(void);
static void stingAttackWindUp(void);
static void stingAttack(void);
static void stingAttackPause(void);
static void fallout(void);
static void ramTouch(Entity *);
static void die(void);
static void dieFinish(void);
static void creditsMove(void);

Entity *addFlyBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Fly Boss");
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

	e->fallout = &fallout;

	e->die = &die;

	e->creditsAction = &creditsMove;

	setEntityAnimation(e, "CUSTOM_1");

	return e;
}

static void initialise()
{
	self->thinkTime++;

	if (self->thinkTime >= 360)
	{
		self->thinkTime = 0;
	}

	if (self->active == TRUE)
	{
		if (self->thinkTime == 1 || self->thinkTime == 181)
		{
			self->thinkTime = 0;
		}

		if (cameraAtMinimum())
		{
			centerMapOnEntity(NULL);

			self->dirX = self->speed;

			self->action = &doIntro;

			self->thinkTime = 180;

			setContinuePoint(FALSE, self->name, NULL);
		}
	}

	self->x = self->startX + sin(DEG_TO_RAD(self->thinkTime)) * 10;
}

static void doIntro()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->x = self->startX;

		setEntityAnimation(self, "STAND");

		playSoundToMap("sound/common/gib", -1, self->x, self->y, 0);

		for (i=0;i<11;i++)
		{
			e = addTemporaryItem("boss/fly_boss_cocoon_piece", self->x, self->y, RIGHT, 0, 0);

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			e->dirX = (prand() % 3) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

			setEntityAnimationByID(e, i);

			e->thinkTime = 180 + (prand() % 60);
		}

		self->draw = &drawLoopingAnimationToMap;

		playSoundToMap("sound/boss/fly_boss/buzz", BOSS_CHANNEL, self->x, self->y, 0);

		self->takeDamage = &takeDamage;

		self->action = &introPause;

		self->touch = &entityTouch;

		self->thinkTime = 120;

		self->startY = self->y;

		self->dirY = self->dirX = 0;

		self->startX = 0;

		facePlayer();
	}

	else
	{
		if (self->x == self->startX || (self->thinkTime % 2 == 0))
		{
			self->x = self->startX + (3 * (self->x < self->startX ? 1 : -1));
		}
	}
}

static void introPause()
{
	hover();

	self->dirX = 0.5;

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		playDefaultBossMusic();

		initBossHealthBar();

		self->action = &entityWait;

		self->flags |= LIMIT_TO_SCREEN;
	}

	facePlayer();
}

static void entityWait()
{
	int i;

	self->thinkTime--;

	facePlayer();

	hover();

	if (self->thinkTime <= 0 && player.health > 0)
	{
		i = self->health <= (self->maxHealth / 10) ? prand() % 10 : prand() % 4;

		switch (i)
		{
			case 0:
				self->action = &bulletFireInit;
			break;

			case 1:
				self->action = &headButtInit;
			break;

			case 2:
				self->thinkTime = 120 + prand() % 180;

				self->action = &dropInit;
			break;

			case 3:
				self->action = &slimeFireInit;
			break;

			default:
				self->action = &stingAttackInit;
			break;
		}

		self->damage = 1;

		playSoundToMap("sound/boss/fly_boss/buzz", BOSS_CHANNEL, self->x, self->y, 0);
	}
}

static int drawSuspended()
{
	drawLine(self->startX + self->w / 2, self->startY, self->x + self->w / 2, self->y + 15, 255, 255, 255);

	drawLoopingAnimationToMap();

	return TRUE;
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

static void flyToTopTarget()
{
	Target *t;

	/* Don't fly through the player, that'd be really annoying */

	if (player.x < self->x)
	{
		t = getTargetByName("FLY_BOSS_TARGET_TOP_RIGHT");
	}

	else
	{
		t = getTargetByName("FLY_BOSS_TARGET_TOP_LEFT");
	}

	if (t == NULL)
	{
		showErrorAndExit("Fly boss cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= self->speed;
	self->dirY *= self->speed;

	self->action = &moveToTarget;
}

static void moveToTarget()
{
	checkToMap(self);

	facePlayer();

	if (atTarget())
	{
		self->thinkTime = 120;

		self->x = self->targetX;
		self->y = self->targetY;

		self->dirX = 0;
		self->dirY = 0;

		self->startX = 0;
		self->startY = self->y;

		self->action = &entityWait;
	}
}

static void dropInit()
{
	Target *left, *right;

	setEntityAnimation(self, "ATTACK_1");

	self->dirY = 0;

	left = getTargetByName("FLY_BOSS_TARGET_TOP_LEFT");

	right = getTargetByName("FLY_BOSS_TARGET_TOP_RIGHT");

	self->thinkTime--;

	self->targetX = player.x - self->w / 2 + player.w / 2;

	if (self->thinkTime > 0)
	{
		/* Move towards player */

		if (abs(self->x - self->targetX) <= self->speed)
		{
			self->dirX = 0;
		}

		else
		{
			self->dirX = self->targetX < self->x ? -self->speed : self->speed;
		}

		checkToMap(self);

		if (self->x < left->x)
		{
			self->x = left->x;

			self->dirX = 0;
		}

		else if (self->x > right->x)
		{
			self->x = right->x;

			self->dirX = 0;
		}
	}

	else
	{
		self->thinkTime = 0;

		self->action = &drop;

		self->dirX = 0;
	}
}

static void drop()
{
	int i;
	long onGround = (self->flags & ON_GROUND);

	self->thinkTime--;

	if (self->thinkTime > 0)
	{
		hover();
	}

	else
	{
		self->frameSpeed = 0;

		self->thinkTime = 0;

		self->flags &= ~FLY;

		checkToMap(self);

		if (onGround == 0 && (self->flags & ON_GROUND))
		{
			playSoundToMap("sound/common/crash", BOSS_CHANNEL, self->x, self->y, 0);

			shakeScreen(LIGHT, 15);

			self->thinkTime = 90;

			self->action = &dropWait;

			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}
		}
	}
}

static void dropWait()
{
	self->thinkTime--;

	facePlayer();

	if (self->thinkTime <= 0)
	{
		self->flags |= FLY;

		self->action = &attackFinished;
	}
}

static void bulletFireInit()
{
	selectRandomBottomTarget();

	self->action = &bulletFireMoveToPosition;
}

static void bulletFireMoveToPosition()
{
	checkToMap(self);

	facePlayer();

	if (atTarget())
	{
		self->maxThinkTime = 5 + prand() % 15;

		self->x = self->targetX;
		self->y = self->targetY;

		self->dirX = 0;
		self->dirY = 0;

		self->startX = 0;
		self->startY = self->y;

		self->action = &fireBullets;
	}
}

static void fireBullets()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime--;

		playSoundToMap("sound/boss/fly_boss/fly_boss_bullet", -1, self->x, self->y, 0);

		e = addProjectile("boss/fly_boss_shot", self, self->x + (self->face == RIGHT ? self->w : 0), self->y + self->h / 2, (self->face == RIGHT ? 7 : -7), 0);

		e->dirY = 0.1 * (prand() % 2 == 0 ? -1 : 1);

		e->reactToBlock = &bounceOffShield;

		if (self->maxThinkTime <= 0)
		{
			self->thinkTime = 120;

			self->action = &attackFinished;
		}

		else
		{
			self->thinkTime = 6;
		}
	}
}

static void slimeFireInit()
{
	selectRandomBottomTarget();

	self->action = &slimeFireMoveToPosition;
}

static void slimeFireMoveToPosition()
{
	checkToMap(self);

	facePlayer();

	if (atTarget())
	{
		self->maxThinkTime = 1 + prand() % 4;

		self->x = self->targetX;
		self->y = self->targetY;

		self->dirX = 0;
		self->dirY = 0;

		self->startX = 0;
		self->startY = self->y;

		self->action = &fireSlime;
	}
}

static void fireSlime()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime--;

		playSoundToMap("sound/boss/grub_boss/fire", -1, self->x, self->y, 0);

		e = addProjectile("boss/fly_boss_slime", self, self->x + (self->face == RIGHT ? self->w : 0), self->y + self->h / 2, (self->face == RIGHT ? 7 : -7), 0);

		e->touch = &slimePlayer;

		if (self->maxThinkTime <= 0)
		{
			self->thinkTime = 60;

			self->action = ((player.flags & HELPLESS) || prand() % 3 == 0) ? &bulletFireMoveToPosition : &attackFinished;
		}

		else
		{
			self->thinkTime = 30;
		}
	}
}

static void headButtInit()
{
	selectRandomBottomTarget();

	self->action = &headButtMoveToPosition;
}

static void headButtMoveToPosition()
{
	checkToMap(self);

	facePlayer();

	if (atTarget())
	{
		self->flags &= ~(FLY|UNBLOCKABLE);

		setEntityAnimation(self, "ATTACK_2");

		self->thinkTime = 60;

		self->x = self->targetX;
		self->y = self->targetY;

		self->dirX = 0;
		self->dirY = 0;

		self->startX = 0;
		self->startY = self->y;

		self->action = &moveToHeadButtRange;
	}
}

static void moveToHeadButtRange()
{
	int playerX, bossX;

	facePlayer();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		bossX = self->x + (self->face == LEFT ? 0 : self->w - 1);

		playerX = player.x + (self->face == RIGHT ? 0 : player.w - 1);

		if (abs(bossX - playerX) < 24)
		{
			self->dirX = self->face == LEFT ? -self->speed * 3 : self->speed * 3;
			self->dirY = -3;

			self->action = &headButt;

			self->reactToBlock = &reactToHeadButtBlock;
		}

		else
		{
			self->dirX = self->face == LEFT ? -self->speed : self->speed;
		}
	}

	checkToMap(self);
}

static void headButt()
{
	self->touch = &ramTouch;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->dirX = (self->face == LEFT ? 4 : -4);
		self->dirY = -6;

		self->action = &headButtFinish;
	}
}

static void headButtFinish()
{
	self->touch = &entityTouch;

	if (self->flags & ON_GROUND)
	{
		facePlayer();

		self->dirX = 0;

		self->thinkTime = 120;

		if (prand() % 2 == 0)
		{
			self->action = &attackFinished;
		}

		else
		{
			self->action = &moveToHeadButtRange;
		}
	}

	checkToMap(self);
}

static void stingAttackInit()
{
	selectRandomBottomTarget();

	self->action = &stingAttackMoveToPosition;
}

static void stingAttackMoveToPosition()
{
	int bottomY;

	checkToMap(self);

	facePlayer();

	if (atTarget())
	{
		self->dirY = 0;

		self->dirX = 0;

		bottomY = self->y + self->h - 1;

		setEntityAnimation(self, "ATTACK_3");

		self->y = bottomY - self->h;

		self->frameSpeed = 12;

		self->thinkTime = 120;

		self->action = &stingAttackWindUp;
	}
}

static void stingAttackWindUp()
{
	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		self->thinkTime = 15;

		self->dirX = self->face == RIGHT ? -4 : 4;

		self->action = &stingAttack;
	}
}

static void stingAttack()
{
	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		self->dirX = self->face == LEFT ? -24 : 24;

		self->flags |= ATTACKING;

		self->touch = &ramTouch;
	}

	else if (self->thinkTime < 0)
	{
		if (self->dirX == 0)
		{
			shakeScreen(MEDIUM, 15);

			self->frameSpeed = 0;

			self->flags &= ~FLY;

			self->dirY = -4;

			self->dirX = self->face == LEFT ? 4 : -4;

			self->action = &stingAttackPause;

			self->touch = &entityTouch;

			setEntityAnimation(self, "ATTACK_2");

			self->thinkTime = 180;
		}
	}

	checkToMap(self);
}

static void stingAttackPause()
{
	self->thinkTime--;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
	}

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}
}

static void fallout()
{
	if (self->environment == WATER)
	{
		self->flags |= HELPLESS;

		self->dirX = 0;

		self->action = &doNothing;
	}
}

static void die()
{
	self->damage = 0;

	self->thinkTime = 120;

	self->flags &= ~FLY;

	self->action = &dieFinish;
}

static void dieFinish()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		clearContinuePoint();

		increaseKillCount();

		freeBossHealthBar();

		e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

		e->dirY = ITEM_JUMP_HEIGHT;

		fadeBossMusic();

		entityDieNoDrop();
	}
}

static void slimePlayer(Entity *other)
{
	if (other->type == PLAYER)
	{
		other->dirX = 0;

		if (!(other->flags & HELPLESS))
		{
			setPlayerSlimed(180);
		}

		self->die();
	}
}

static void selectRandomBottomTarget()
{
	Target *t;

	if (prand() % 2 == 0)
	{
		t = getTargetByName("FLY_BOSS_TARGET_BOTTOM_RIGHT");
	}

	else
	{
		t = getTargetByName("FLY_BOSS_TARGET_BOTTOM_LEFT");
	}

	if (t == NULL)
	{
		showErrorAndExit("Fly boss cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= self->speed;
	self->dirY *= self->speed;

	facePlayer();
}

static void reactToHeadButtBlock(Entity *other)
{
	self->dirX = 0;
}

static void takeDamage(Entity *other, int damage)
{
	int minHealth;
	Entity *temp;

	minHealth = self->maxHealth / 10;

	if (!(self->flags & INVULNERABLE))
	{
		/* Take minimal damage from bombs */

		if (other->type == EXPLOSION)
		{
			damage = 1;
		}

		self->health -= damage;

		if (self->health <= minHealth)
		{
			self->health = minHealth;
		}

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		enemyPain();

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}
	}
}

static void attackFinished()
{
	int bottomY;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->frameSpeed = 1;

		bottomY = self->y + self->h - 1;

		self->frameSpeed = 1;

		setEntityAnimation(self, "STAND");

		self->y = bottomY - self->h - 1;

		self->dirX = self->face == RIGHT ? -1 : 1;
		self->dirY = 1;

		self->startX = 0;
		self->startY = self->y;

		self->flags &= ~ATTACKING;

		self->flags |= UNBLOCKABLE|FLY;

		/* Stop the player from being hit when the animation changes */

		self->damage = 0;

		self->action = &flyToTopTarget;

		self->reactToBlock = NULL;

		self->thinkTime = 30;
	}

	checkToMap(self);
}

static void ramTouch(Entity *other)
{
	int health = player.health;

	entityTouch(other);

	if (player.health < health)
	{
		reactToHeadButtBlock(other);
	}
}

static void creditsMove()
{
	setEntityAnimation(self, "STAND");

	self->draw = &drawLoopingAnimationToMap;

	self->creditsAction = &bossMoveToMiddle;
}
