/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../audio/music.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../graphics/decoration.h"
#include "../game.h"
#include "../hud.h"
#include "../map.h"
#include "../item/key_items.h"
#include "../player.h"
#include "../enemy/enemies.h"
#include "../graphics/gib.h"
#include "../system/error.h"
#include "../world/target.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void die(void);
static void attackFinished(void);
static void takeDamage(Entity *, int);
static void touch(Entity *);
static void entityWait(void);
static void changeToIceInit(void);
static void changeToFireInit(void);
static void changeToFire(void);
static void throwWallWalkerInit(void);
static void throwWallWalker(void);
static void rechargeInit(void);
static void moveToRechargeTarget(void);
static void recharge(void);
static void incinerateInit(void);
static void incinerate(void);
static void incinerateWait(void);
static void fireWait(void);
static int fireDraw(void);
static void spitFireInit(void);
static void spitFireMoveToTarget(void);
static void spitFire(void);

Entity *addCaveBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Cave Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;

	e->die = &die;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	if (self->active == TRUE)
	{
		self->flags &= ~NO_DRAW;

		if (cameraAtMinimum())
		{
			centerMapOnEntity(NULL);

			self->action = &doIntro;

			self->thinkTime = 60;

			self->touch = &touch;

			setContinuePoint(FALSE, self->name, NULL);
		}
	}

	checkToMap(self);
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= LIMIT_TO_SCREEN;

		playDefaultBossMusic();

		initBossHealthBar();

		self->takeDamage = &takeDamage;

		self->action = &entityWait;

		checkToMap(self);

		self->startX = 0;

		self->endX = 0;

		self->thinkTime = 0;
	}

	checkToMap(self);
}

static void entityWait()
{
	int action;

	if (self->startX == 0)
	{
		if (self->endX == 0)
		{
			self->action = &changeToFireInit;
		}

		else if (self->endX == 1)
		{
			self->action = &changeToIceInit;
		}

		else
		{
			self->startX = 1;
		}

		checkToMap(self);
	}

	else
	{
		self->action = &spitFireInit;
		
		return;
		
		switch ((int)self->endX)
		{
			case 0:
				setEntityAnimation(self, CUSTOM_2);
			break;

			case 1:
				setEntityAnimation(self, CUSTOM_4);
			break;

			default:
				setEntityAnimation(self, WALK);
			break;
		}

		moveLeftToRight();

		self->thinkTime--;

		if (self->thinkTime <= 0 && player.health > 0)
		{
			switch ((int)self->startX)
			{
				case 1: /* Fire */
					action = prand() % 3;

					switch (action)
					{
						case 0:
							self->action = &throwWallWalkerInit;
						break;

						case 1:
							self->action = &incinerateInit;
						break;
					}
				break;

				default: /* Ice */
					action = prand() % 3;

					switch (action)
					{
						case 0:
							self->action = &throwWallWalkerInit;
						break;
					}
				break;
			}
		}
	}
}

static void spitFireInit()
{
	Target *t = getTargetByName(prand() % 2 == 0 ? "CAVE_BOSS_TARGET_LEFT" : "CAVE_BOSS_TARGET_RIGHT");
	
	if (t == NULL)
	{
		showErrorAndExit("Cave Boss cannot find target");
	}
	
	self->targetX = t->x;
	
	self->mental = 5;
	
	if (self->x == self->targetX && self->y == self->targetY)
	{
		self->action = &spitFire;
	}
	
	else
	{
		t = getTargetByName("CAVE_BOSS_TARGET_TOP");
		
		if (t == NULL)
		{
			showErrorAndExit("Cave Boss cannot find target");
		}
		
		self->targetY = t->y;
		
		self->action = &spitFireMoveToTarget;
		
		if (self->startX == 1)
		{
			setEntityAnimation(self, DIE);
		}
		
		else if (self->startX == 2)
		{
			
		}
		
		else
		{
			setEntityAnimation(self, JUMP);
		}
		
		self->dirX = 0;
		self->dirY = -self->speed;
		
		printf("Moving up\n");
	}
}

static void spitFireMoveToTarget()
{
	Target *t;
	
	checkToMap(self);
	
	if (self->dirY < 0 && self->y <= self->targetY)
	{
		self->y = self->targetY;
		
		self->dirY = 0;
		
		self->face = self->targetX < self->x ? LEFT : RIGHT;
		
		self->dirX = self->face == LEFT ? -self->speed : self->speed;
		
		if (self->startX == 1)
		{
			setEntityAnimation(self, PAIN);
		}
		
		else if (self->startX == 2)
		{
			
		}
		
		else
		{
			setEntityAnimation(self, WALK);
		}
		
		printf("Moving horiz\n");
	}
	
	else if ((self->dirX < 0 && self->x <= self->targetX) || (self->dirX > 0 && self->x >= self->targetX))
	{
		t = getTargetByName(self->dirX < 0 ? "CAVE_BOSS_TARGET_LEFT" : "CAVE_BOSS_TARGET_RIGHT");
		
		if (t == NULL)
		{
			showErrorAndExit("Cave Boss cannot find target");
		}
		
		self->x = self->targetX;
		
		self->targetY = t->y;
		
		self->face = self->dirX < 0 ? RIGHT : LEFT;
		
		self->dirX = 0;
		
		self->dirY = self->speed;
		
		if (self->startX == 1)
		{
			setEntityAnimation(self, DIE);
		}
		
		else if (self->startX == 2)
		{
			
		}
		
		else
		{
			setEntityAnimation(self, JUMP);
		}
		
		printf("Moving down\n");
	}
	
	else if (self->dirY > 0 && self->y >= self->targetY)
	{
		self->dirY = 0;
		
		if (self->startX == 1)
		{
			setEntityAnimation(self, CUSTOM_1);
		}
		
		else if (self->startX == 2)
		{
			
		}
		
		else
		{
			setEntityAnimation(self, STAND);
		}
		
		self->action = &spitFire;
		
		facePlayer();
		
		self->thinkTime = 60;
		
		printf("At target\n");
	}
}

static void spitFire()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}
}

static void rechargeInit()
{
	Target *t;

	t = getTargetByName(prand() % 2 == 0 ? "CAVE_BOSS_TARGET_LEFT" : "CAVE_BOSS_TARGET_RIGHT");

	if (t == NULL)
	{
		showErrorAndExit("Cave Boss cannot find target");
	}

	self->targetX = t->x;

	self->targetY = self->y;

	self->action = &moveToRechargeTarget;

	self->face = self->targetX < self->x ? LEFT : RIGHT;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	checkToMap(self);
}

static void moveToRechargeTarget()
{
	if (atTarget())
	{
		self->thinkTime = 600;

		self->action = &recharge;
	}

	checkToMap(self);
}

static void recharge()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void throwWallWalkerInit()
{
	self->mental = 1 + prand() % 5;

	self->thinkTime = 30;

	self->action = &throwWallWalker;

	checkToMap(self);
}

static void throwWallWalker()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addEnemy("enemy/wall_walker", self->x, self->y);

		e->face = self->face;

		e->dirY = ITEM_JUMP_HEIGHT;

		e->flags |= LIMIT_TO_SCREEN;

		e->dirX = e->face == LEFT ? -e->speed : e->speed;

		self->mental--;

		self->thinkTime = 60;

		if (self->mental <= 0)
		{
			self->action = &attackFinished;
		}
	}

	checkToMap(self);
}

static void changeToIceInit()
{
	self->startX = 2;

	self->maxThinkTime = 16;

	playSoundToMap("sound/common/freeze.ogg", BOSS_CHANNEL, self->x, self->y, 0);

	self->action = &attackFinished;
}

static void changeToFireInit()
{
	self->startX = 1;

	playSoundToMap("sound/enemy/fireball/fireball.ogg", BOSS_CHANNEL, self->x, self->y, 0);

	self->action = &changeToFire;
	
	self->draw = &fireDraw;
}

static void changeToFire()
{
	self->alpha -= 3;

	if (self->alpha <= 0)
	{
		setEntityAnimation(self, CUSTOM_1);

		self->alpha = 255;

		self->action = &attackFinished;

		self->draw = &drawLoopingAnimationToMap;

		self->mental = 0;
		
		self->maxThinkTime = 10;
	}

	checkToMap(self);
}

static void attackFinished()
{
	self->mental = 0;

	self->thinkTime = 120;

	self->action = &entityWait;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (self->startX != -1)
	{
		playSoundToMap("sound/common/dink.ogg", EDGAR_CHANNEL, self->x, self->y, 0);

		if (other->reactToBlock != NULL)
		{
			temp = self;

			self = other;

			self->reactToBlock(temp);

			self = temp;
		}

		if (prand() % 10 == 0)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
		}

		damage = 0;

		addDamageScore(damage, self);

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
	}

	else
	{
		entityTakeDamageNoFlinch(other, damage);
	}
}

static void die()
{
	Entity *e;

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		clearContinuePoint();

		increaseKillCount();

		freeBossHealthBar();

		e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

		e->dirY = ITEM_JUMP_HEIGHT;

		entityDieNoDrop();
	}
}

static void touch(Entity *other)
{
	if (self->startX == -1 && other->type == KEY_ITEM && strcmpignorecase(other->name, "item_stalactite") == 0)
	{
		self->takeDamage(other, 500);
	}

	else if (self->endX == 1 && other->type == KEY_ITEM && strcmpignorecase(other->name, "item/ice_cube") == 0)
	{
		self->maxThinkTime--;

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		enemyPain();

		other->inUse = FALSE;

		if (self->maxThinkTime <= 0)
		{
			self->maxThinkTime = 150;

			self->startX = -1;

			self->action = &rechargeInit;
		}
	}

	else if (self->endX == 2 && other->type == PROJECTILE && strcmpignorecase(other->name, "weapon/flaming_arrow") == 0)
	{
		self->maxThinkTime--;

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		enemyPain();

		other->inUse = FALSE;

		if (self->maxThinkTime <= 0)
		{
			self->maxThinkTime = 150;

			self->startX = -1;

			self->action = &rechargeInit;
		}
	}

	else
	{
		entityTouch(other);
	}
}

static void incinerateInit()
{
	self->thinkTime = 300;

	self->action = &incinerate;

	self->mental = 60;

	checkToMap(self);
}

static void incinerate()
{
	int x, startX;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental = 0;

		startX = getMapStartX();

		x = 0;

		while (x < SCREEN_WIDTH)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add the Fire");
			}

			loadProperties("boss/phoenix_die_fire", e);

			setEntityAnimation(e, STAND);

			e->x = startX + x;
			e->y = self->y + self->h - e->h;

			e->action = &fireWait;
			e->touch = &entityTouch;

			e->draw = &drawLoopingAnimationToMap;

			e->type = ENEMY;

			e->flags |= FLY;

			e->thinkTime = 30;

			e->damage = 1;

			e->health = 0;

			e->maxHealth = 5;

			e->mental = 1;

			e->head = self;

			x += e->w;

			self->mental++;
		}

		self->action = &incinerateWait;
	}

	checkToMap(self);
}

static void incinerateWait()
{
	if (self->mental <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void fireWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->health += self->mental;

		if (self->health == self->maxHealth)
		{
			self->maxHealth = 5;

			self->thinkTime = 180;

			self->mental *= -1;
		}

		else if (self->health < 0)
		{
			self->head->mental--;

			self->inUse = FALSE;

			self->health = 0;
		}

		else
		{
			self->thinkTime = 5;
		}

		setEntityAnimation(self, self->health);
	}

	checkToMap(self);
}

static int fireDraw()
{
	int frame, alpha;
	float timer;

	/* Draw the boss with its lowering alpha */

	drawLoopingAnimationToMap();

	frame = self->currentFrame;
	timer = self->frameTimer;

	alpha = self->alpha;

	/* Draw the other part with its rising alpha */

	setEntityAnimation(self, CUSTOM_1);

	self->currentFrame = frame;
	self->frameTimer = timer;

	self->alpha = 255 - alpha;

	drawSpriteToMap();

	/* Reset back to original */

	setEntityAnimation(self, STAND);

	self->currentFrame = frame;
	self->frameTimer = timer;

	self->alpha = alpha;

	return 1;
}
