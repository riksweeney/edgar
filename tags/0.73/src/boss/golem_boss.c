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
#include "../medal.h"
#include "../enemy/enemies.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../map.h"
#include "../audio/music.h"
#include "../event/trigger.h"
#include "../item/key_items.h"
#include "../item/item.h"
#include "../collisions.h"
#include "../event/script.h"
#include "../custom_actions.h"
#include "../hud.h"
#include "../game.h"
#include "../player.h"
#include "../geometry.h"
#include "../graphics/decoration.h"
#include "../world/target.h"
#include "../enemy/rock.h"
#include "../system/error.h"

extern Entity *self, player, entity[MAX_ENTITIES];

static void initialise(void);
static void headReform(void);
static void headReform2(void);
static void reform(void);
static void reform2(void);
static void shatter(void);
static void initialShatter(void);
static void die(void);
static void takeDamage(Entity *, int);
static void shatter(void);
static void reform(void);
static void headReform(void);
static void reform2(void);
static void headReform2(void);
static void commence(void);
static void entityWait(void);
static void headWait(void);
static void partWait(void);
static void attackFinished(void);
static void stompAttackStart(void);
static void stompAttack(void);
static void stompShake(void);
static void stompAttackFinish(void);
static void attackFinished(void);
static void stunnedTouch(Entity *);
static void die(void);
static void dieFinish(void);
static void throwRockStart(void);
static void throwRock(void);
static void throwRockFinish(void);
static void rockWait(void);
static void jumpAttackStart(void);
static void jumpAttack(void);
static void rockTouch(Entity *);
static void rockPush(Entity *);
static void explodeOnGround(void);

Entity *addGolemBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Golem Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialShatter;

	e->draw = &drawLoopingAnimationToMap;
	e->takeDamage = &takeDamage;
	e->die = &die;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialShatter()
{
	self->y = self->startY;

	shatter();

	self->action = &initialise;

	self->touch = NULL;
}

static void shatter()
{
	int i;
	float y;
	Entity *e, *previous;

	self->dirX = 0;
	self->dirY = 0;

	self->targetX = self->x;
	self->targetY = self->startY;

	self->animationCallback = NULL;
	
	y = self->y;

	setEntityAnimation(self, CUSTOM_1);
	
	self->y = y;

	self->maxThinkTime = 14;

	previous = self;

	if (self->target != NULL)
	{
		e = self;

		self = e->target;

		self->die();

		self = e;

		self->target = NULL;
	}

	for (i=0;i<self->maxThinkTime;i++)
	{
		e = getFreeEntity();

		loadProperties("boss/golem_boss_piece", e);

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Golem Boss Body Part");
		}

		switch (i)
		{
			case 0: /* Back hand */
				e->targetX = 28;
				e->targetY = 90;
			break;

			case 1: /* Back forearm  */
				e->targetX = 29;
				e->targetY = 53;
			break;

			case 2: /* Back shoulder  */
				e->targetX = 28;
				e->targetY = 36;
			break;

			case 3: /* Back foot  */
				e->targetX = 31;
				e->targetY = 155;
			break;

			case 4: /* Back leg  */
				e->targetX = 31;
				e->targetY = 108;
			break;

			case 5: /* Back hip joint  */
				e->targetX = 20;
				e->targetY = 95;
			break;

			case 6: /* Torso  */
				e->targetX = 7;
				e->targetY = 21;
			break;

			case 7: /* Hip  */
				e->targetX = 17;
				e->targetY = 78;
			break;

			case 8: /* Front hip joint  */
				e->targetX = 20;
				e->targetY = 95;
			break;

			case 9: /* Front leg  */
				e->targetX = 23;
				e->targetY = 108;
			break;

			case 10: /* Front foot  */
				e->targetX = 23;
				e->targetY = 155;
			break;

			case 11: /* Front shoulder  */
				e->targetX = 28;
				e->targetY = 36;
			break;

			case 12: /* Front forearm  */
				e->targetX = 29;
				e->targetY = 52;
			break;

			case 13: /* Front hand  */
				e->targetX = 28;
				e->targetY = 90;
			break;
		}

		setEntityAnimation(e, i);

		e->action = &partWait;

		e->draw = &drawLoopingAnimationToMap;

		e->dirX = 1 + prand() % 12 * (prand() % 2 == 0 ? -1 : 1);

		e->dirY = -prand() % 6;

		previous->target = e;

		e->head = self;

		e->x = self->x;

		e->y = self->y;

		e->face = self->face;

		e->thinkTime = prand() % 60;
		
		e->health = 360;

		e->touch = &entityTouch;

		e->damage = 0;

		if (e->face == LEFT)
		{
			e->targetX = self->x + self->w - e->w - e->targetX;
		}

		else
		{
			e->targetX = self->x + e->targetX;
		}

		e->targetY += self->startY;

		e->target = NULL;

		previous = e;
	}

	self->touch = &stunnedTouch;

	self->thinkTime = 300;

	self->action = &headWait;
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

			self->thinkTime = 0;

			self->action = &headWait;
			
			setContinuePoint(FALSE, self->name, NULL);
		}
	}

	checkToMap(self);
}

static void commence()
{
	playDefaultBossMusic();

	initBossHealthBar();

	self->action = &entityWait;
}

static void entityWait()
{
	int attack;

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		attack = prand() % 3;

		switch (attack)
		{
			case 0:
				self->maxThinkTime = 5;

				self->action = &throwRockStart;
			break;

			case 1:
				self->action = &stompAttackStart;
			break;

			default:
				self->action = &jumpAttackStart;
			break;
		}
	}

	facePlayer();

	checkToMap(self);
}

static void stompAttackStart()
{
	setEntityAnimation(self, ATTACK_1);

	self->dirX = 0;

	self->action = &stompAttack;

	self->animationCallback = &stompShake;

	checkToMap(self);
}

static void stompAttack()
{
	checkToMap(self);
}

static void stompShake()
{
	setEntityAnimation(self, ATTACK_2);

	playSoundToMap("sound/common/crash.ogg", BOSS_CHANNEL, self->x, self->y, 0);

	shakeScreen(STRONG, 120);

	activateEntitiesValueWithObjectiveName("GOLEM_ROCK_DROPPER", 5);

	self->frameSpeed = 0;

	self->thinkTime = 120;

	if (player.flags & ON_GROUND)
	{
		setPlayerStunned(120);
	}

	self->action = &stompAttackFinish;
}

static void stompAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void attackFinished()
{
	self->frameSpeed = 1;

	setEntityAnimation(self, STAND);

	self->thinkTime = 60;

	self->action = &entityWait;

	checkToMap(self);
}

static void stunnedTouch(Entity *other)
{
	/* Player does not take damage from touching the Golem */

	if (other->type == PLAYER)
	{
		pushEntity(other);
	}

	else if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage * 3);
		}
	}
}

static void takeDamage(Entity *other, int damage)
{
	int i, health;
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	health = self->health;

	if (strcmpignorecase(other->name, "weapon/pickaxe") == 0)
	{
		self->health -= damage;

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);
			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

			enemyPain();
		}

		else
		{
			self->health = 19;

			setCustomAction(self, &flashWhite, 6, 0, 0);
			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

			/* Don't reshatter */

			if (self->currentAnim != self->animation[CUSTOM_1])
			{
				self->action = &shatter;

				self->thinkTime = 300;
			}
		}

		self->mental = 1;
	}

	else if (other->type == PROJECTILE && strcmpignorecase(other->name, "enemy/small_boulder") == 0)
	{
		self->health -= damage;

		if (self->health < 0)
		{
			self->health = 19;
		}

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		self->action = &shatter;

		self->thinkTime = 300;

		temp = self;

		self = other;

		self->die();

		self = temp;
	}

	else
	{
		playSoundToMap("sound/common/dink.ogg", EDGAR_CHANNEL, self->x, self->y, 0);
		
		if (other->reactToBlock != NULL)
		{
			temp = self;

			self = other;

			self->reactToBlock();

			self = temp;
		}

		if (prand() % 10 == 0)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
		}

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
		
		damage = 0;
	}

	if (health > 20 && self->health <= 20)
	{
		for (i=0;i<MAX_ENTITIES;i++)
		{
			if (entity[i].inUse == TRUE && strcmpignorecase(entity[i].objectiveName, "ROCK_GRABBER") == 0)
			{
				entity[i].health = 1;
			}
		}
	}
	
	addDamageScore(damage, self);
}

static void throwRockStart()
{
	/* Crouch to pick up rock */

	setEntityAnimation(self, ATTACK_3);

	self->animationCallback = &throwRock;

	checkToMap(self);
}

static void throwRock()
{
	Entity *e;

	e = addEnemy("enemy/small_boulder", self->x, self->y);

	e->x += self->face == RIGHT ? self->w : -e->w;

	e->dirX = 0;
	e->dirY = 0;

	e->face = RIGHT;

	e->frameSpeed = 0;

	e->flags |= FLY;

	e->action = &rockWait;

	e->flags &= ~FLY;

	e->dirX = self->face == LEFT ? -2 * self->maxThinkTime : 2 * self->maxThinkTime;

	e->dirY = -10;

	e->touch = &rockTouch;

	setEntityAnimation(self, ATTACK_4);

	self->thinkTime = 60;

	self->action = &throwRockFinish;
}

static void throwRockFinish()
{
	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		self->maxThinkTime--;

		if (self->maxThinkTime <= 0)
		{
			self->action = &attackFinished;
		}

		else
		{
			setEntityAnimation(self, ATTACK_5);

			self->animationCallback = &throwRock;
		}
	}

	checkToMap(self);
}

static void jumpAttackStart()
{
	self->maxThinkTime = 4;

	/* First jump is on the spot */

	self->dirY =- 13;

	self->dirX = 0;

	self->action = &jumpAttack;

	self->thinkTime = 30;

	checkToMap(self);
}

static void jumpAttack()
{
	int i;
	int wasOnGround = self->flags & ON_GROUND;
	float dirX;
	Entity *e;

	dirX = self->dirX;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (wasOnGround == 0)
		{
			/* Stop if you hit the wall while jumping */

			if (self->dirX == 0 && self->maxThinkTime != 4)
			{
				self->maxThinkTime = 0;
			}

			else
			{
				self->maxThinkTime--;
			}

			self->dirX = 0;

			shakeScreen(MEDIUM, 30);

			playSoundToMap("sound/common/crash.ogg", BOSS_CHANNEL, self->x, self->y, 0);

			for (i=0;i<30;i++)
			{
				e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

				if (e != NULL)
				{
					e->y -= prand() % e->h;
				}
			}

			self->thinkTime = 30;

			if (self->maxThinkTime <= 0)
			{
				self->action = &attackFinished;
			}
		}

		else
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->dirY =- 13;

				self->dirX = self->face == LEFT ? -6 : 6;

				self->action = &jumpAttack;
			}
		}
	}
}

static void die()
{
	Entity *e;

	if (self->active == TRUE)
	{
		/* Disable all the parts */

		self->damage = 0;

		e = self->target;

		while (e != NULL)
		{
			e->action = &partWait;

			e->dirX = 0;

			e->dirY = 0;

			e->flags &= ~FLY;

			e = e->target;
		}

		shakeScreen(MEDIUM, 0);

		stopSound(BOSS_CHANNEL);

		self->active = FALSE;

		self->action = &headWait;

		self->die = &dieFinish;

		self->resumeNormalFunction = &explodeOnGround;

		runScript("golem_boss_die");
	}
}

static void explodeOnGround()
{
	int i;
	Entity *e;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		for (i=0;i<20;i++)
		{
			e = addSmallRock(self->x, self->y, "common/small_rock");

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			e->dirX = (1 + prand() % 50) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = -7 - prand() % 5;

			e->dirX /= 10;
		}

		self->touch = NULL;

		self->flags |= NO_DRAW;

		self->active = TRUE;

		self->thinkTime = 120;

		self->action = &dieFinish;
	}
}

static void dieFinish()
{
	Entity *e;

	if (self->active == TRUE)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->mental == 0)
			{
				addMedal("golem_kill_grabbers");
			}
			
			clearContinuePoint();

			increaseKillCount();

			freeBossHealthBar();

			e = addKeyItem("item/heart_container", self->x, self->y);

			e->x += (self->w - e->w) / 2;

			e->dirY = ITEM_JUMP_HEIGHT;

			fadeBossMusic();

			e = self->target;

			while (e != NULL)
			{
				e->thinkTime = 60 + prand() % 120;

				e->action = &generalItemAction;

				e = e->target;
			}

			self->inUse = FALSE;

			runScript("golem_boss_die_finish");
		}
	}
}

static void reform()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		/* Move towards the head */
		
		self->health--;

		if (fabs(self->x - self->targetX) <= fabs(self->dirX) || self->health <= 0)
		{
			self->dirX = 0;
			
			self->x = self->targetX;

			self->head->maxThinkTime--;

			self->action = &partWait;
		}

		else
		{
			self->dirX = self->targetX > self->x ? self->speed : -self->speed;
		}
	}

	checkToMap(self);
}

static void headReform()
{
	Entity *e;
	float speed;

	if (self->maxThinkTime == 0)
	{
		self->action = &headReform2;

		self->flags |= FLY;

		speed = 5 + prand() % 15;

		speed /= 10;

		self->dirY = -speed;

		e = self->target;

		while (e != NULL)
		{
			self->maxThinkTime++;

			e->action = &reform2;

			e->flags |= FLY;

			speed = 5 + prand() % 15;

			speed /= 10;

			e->dirY = -speed;

			e = e->target;
		}
	}

	checkToMap(self);
}

static void reform2()
{
	if (self->y <= self->targetY)
	{
		self->dirY = 0;

		self->y = self->targetY;

		self->head->maxThinkTime--;

		self->action = &partWait;
	}

	checkToMap(self);
}

static void headReform2()
{
	Entity *e;

	if (self->y <= self->targetY)
	{
		self->dirY = 0;

		self->y = self->targetY;

		if (self->maxThinkTime == 0)
		{
			self->thinkTime = 90;

			if (self->health == self->maxHealth)
			{
				self->action = &commence;
			}

			else
			{
				self->action = &entityWait;
			}

			setEntityAnimation(self, STAND);
			
			self->y = self->startY;

			self->flags &= ~FLY;

			e = self->target;

			while (e != NULL)
			{
				e->inUse = FALSE;

				e = e->target;
			}

			self->target = NULL;

			shakeScreen(MEDIUM, 30);

			stopSound(BOSS_CHANNEL);

			self->touch = &entityTouch;
		}
	}

	checkToMap(self);
}

static void partWait()
{
	int i;
	long onGround = self->flags & ON_GROUND;
	Entity *e;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		if (onGround == 0)
		{
			for (i=0;i<5;i++)
			{
				e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

				if (e != NULL)
				{
					e->y -= prand() % e->h;
				}
			}
		}
	}
}

static void headWait()
{
	int i;
	long onGround = self->flags & ON_GROUND;
	Entity *e;

	if (self->active == TRUE)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			playSoundToMap("sound/boss/boulder_boss/roll.ogg", BOSS_CHANNEL, self->x, self->y, -1);

			shakeScreen(MEDIUM, -1);

			self->action = &headReform;

			e = self->target;

			while (e != NULL)
			{
				e->action = &reform;

				e = e->target;
			}
		}
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (onGround == 0)
		{
			for (i=0;i<5;i++)
			{
				e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

				if (e != NULL)
				{
					e->y -= prand() % e->h;
				}
			}
		}
	}
}

static void rockWait()
{
	float dirX = self->dirX;

	checkToMap(self);

	if ((self->flags & ON_GROUND) || (self->dirX == 0 && dirX != 0 && !(self->flags & ON_GROUND)))
	{
		if (prand() % 3 == 0)
		{
			self->action = &doNothing;

			self->flags |= PUSHABLE;

			self->touch = &rockPush;
		}

		else
		{
			self->die();
		}
	}
}

static void rockTouch(Entity *other)
{
	Entity *temp;

	if (other->type == PLAYER)
	{
		temp = self;

		self = other;

		self->takeDamage(temp, temp->damage);

		self = temp;

		self->die();
	}
}

static void rockPush(Entity *other)
{
	if (other->type == PLAYER)
	{
		pushEntity(other);
	}

	else if (other->type == ENEMY)
	{
		self->die();
	}
}
