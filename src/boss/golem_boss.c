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
#include "../enemy/enemies.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../map.h"
#include "../audio/music.h"
#include "../event/trigger.h"
#include "../item/key_items.h"
#include "../collisions.h"
#include "../event/script.h"
#include "../custom_actions.h"
#include "../hud.h"
#include "../game.h"
#include "../player.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void die(void);
static void takeDamage(Entity *, int);
static void doIntro(void);
static void walkIn(void);
static void roar(void);
static void commence(void);
static void wait(void);
static void attackFinished(void);
static void stompAttackStart(void);
static void stompAttack(void);
static void stompShake(void);
static void stompAttackFinish(void);
static void attackFinished(void);
static void stunnedTouch(Entity *);
static void takeDamage(Entity *, int);
static void stunned(void);
static void standUp(void);
static void die(void);
static void dieFinish(void);
static void throwRockStart(void);
static void throwRock(void);
static void rockWait(void);
static void jumpAttackStart(void);
static void jumpAttack(void);

Entity *addGolemBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add the Golem Boss\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->takeDamage = &takeDamage;
	e->die = &die;
	e->touch = &entityTouch;

	e->type = ENEMY;

	e->flags |= NO_DRAW|FLY;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	int minX, minY;

	minX = getMapStartX();
	minY = getMapStartY();

	self->flags |= NO_DRAW;

	if (self->active == TRUE)
	{
		if (cameraAtMinimum())
		{
			centerMapOnEntity(NULL);

			self->action = &doIntro;

			self->maxThinkTime = 3;

			self->thinkTime = 45;
		}
	}
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		printf("Crunch\n");

		self->maxThinkTime--;

		if (self->maxThinkTime == 0)
		{
			setEntityAnimation(self, WALK);

			self->action = &walkIn;

			self->targetX = self->x - 640;

			self->thinkTime = 60;
		}

		else
		{
			self->thinkTime = 45;
		}
	}

	checkToMap(self);
}

static void walkIn()
{
	if (fabs(self->x - self->targetX) <= fabs(self->dirX))
	{
		setEntityAnimation(self, STAND);

		self->dirX = 0;

		self->thinkTime = 60;

		self->action = &roar;
	}

	checkToMap(self);
}

static void roar()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		printf("Roar!\n");

		self->resumeNormalFunction = &commence;

		runScript("golem_intro_done");
	}
}

static void commence()
{
	playBossMusic();

	initBossHealthBar();

	self->action = &wait;
}

static void wait()
{
	int attack = prand() % 3;

	switch (attack)
	{
		case 0:
			setEntityAnimation(self, ATTACK_2);

			self->thinkTime = 90;

			self->action = &throwRockStart;
		break;

		case 1:
			self->action = &stompAttackStart;
		break;

		default:
			self->action = &jumpAttackStart;
		break;
	}

	checkToMap(self);
}

static void stompAttackStart()
{
	setEntityAnimation(self, ATTACK_1);

	self->dirX = 0;

	self->frameSpeed = 0;

	self->thinkTime = 90;

	self->action = &stompAttack;

	self->animationCallback = stompShake;

	checkToMap(self);
}

static void stompAttack()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->frameSpeed = 1;

		self->thinkTime = 180;
	}

	checkToMap(self);
}

static void stompShake()
{
	shakeScreen(STRONG, 120);

	self->frameSpeed = 0;

	if (player.flags & ON_GROUND)
	{
		setPlayerStunned(120);
	}

	self->action = &stompAttackFinish;

	activateEntitiesValueWithObjectiveName("GOLEM_ROCK_DROPPER", 10);
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
	self->frameSpeed = 0;

	setEntityAnimation(self, STAND);

	self->action = &wait;

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
			self->takeDamage(other, other->damage);
		}
	}
}

static void takeDamage(Entity *other, int damage)
{
	int health;
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (strcmpignorecase(other->name, "weapon/pickaxe") == 0)
	{
		health = self->health;

		self->health -= damage;

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);

			if (self->pain != NULL)
			{
				self->pain();
			}
		}

		else
		{
			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);

			self->touch = &stunnedTouch;

			self->action = &stunned;

			setEntityAnimation(self, PAIN);

			self->thinkTime = 300;
		}
	}

	else if (other->type == PROJECTILE && strcmpignorecase(other->name, "enemy/small_boulder") == 0)
	{
		self->health -= damage;

		setCustomAction(self, &flashWhite, 6, 0);
		setCustomAction(self, &invulnerableNoFlash, 20, 0);

		self->touch = &stunnedTouch;

		self->action = &stunned;

		setEntityAnimation(self, PAIN);

		self->thinkTime = 300;

		temp = self;

		self = other;

		self->die();

		self = temp;
	}

	else
	{
		playSoundToMap("sound/common/dink.ogg", EDGAR_CHANNEL, self->x, self->y, 0);

		if (prand() % 10 == 0)
		{
			setInfoBoxMessage(60,  _("This weapon is not having any effect..."));
		}

		setCustomAction(self, &invulnerableNoFlash, 20, 0);
	}

	if (health > 10 && self->health <= 10)
	{
		runScript("golem_grabbers");
	}
}

static void stunned()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, CUSTOM_1);

		self->touch = &entityTouch;

		self->animationCallback = &standUp;
	}

	self->damage = 0;
}

static void standUp()
{
	self->damage = 1;

	self->action = &attackFinished;
}

static void throwRockStart()
{
	/* Crouch to pick up rock */

	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addEnemy("enemy/large_boulder", self->x, self->y);

		e->frameSpeed = 0;

		e->action = &rockWait;

		self->thinkTime = 120;

		self->target = e;

		self->action = &throwRock;
	}
}

static void throwRock()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->target->dirX = self->face == LEFT ? -6 : 6;

		self->target->dirY = -10;

		self->target = NULL;

		self->action = &attackFinished;
	}
}

static void rockWait()
{
	checkToMap(self);

	if ((self->flags & ON_GROUND))
	{
		self->action = &doNothing;

		self->touch = &pushEntity;
	}
}

static void jumpAttackStart()
{
	self->maxThinkTime = 3;

	self->dirY =- 10;

	self->dirX = self->face == LEFT ? -6 : 6;

	self->action = &jumpAttack;

	self->thinkTime = 30;

	checkToMap(self);
}

static void jumpAttack()
{
	int wasOnGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (wasOnGround == 0)
		{
			self->maxThinkTime--;

			self->dirX = 0;

			shakeScreen(15, MEDIUM);

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
				self->dirY =- 10;

				self->dirX = self->face == LEFT ? -6 : 6;

				self->action = &jumpAttack;
			}
		}
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
		freeBossHealthBar();

		e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

		e->dirY = ITEM_JUMP_HEIGHT;

		fadeBossMusic();

		entityDieNoDrop();
	}
}
