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

extern Entity *self, player;
extern Game game;

static void initialise(void);
static void doIntro(void);
static void entityWait(void);
static void attackFinished(void);
static void swordStabInit(void);
static void swordStab(void);
static void swordStabWait(void);
static void swordMoveUnderPlayer(void);
static void swordRise(void);
static void swordSink(void);
static void swordFinish(void);
static void swordTakeDamage(Entity *, int);
static void swordDie(void);
static void swordDropInit(void);
static void swordDrop(void);
static void swordDropFinish(void);
static void takeDamage(Entity *, int);
static void armourTakeDamage(Entity *, int);
static void shieldWait(void);
static void shieldBiteInit(void);
static void shieldBiteWait(void);
static void shieldBite(void);
static void shieldBiteReactToBlock(Entity *);
static void shieldBiteReturn(void);
static void shieldAttackFinish(void);
static void shieldFlameAttack(void);
static void shieldFlameWait(void);
static void shieldTakeDamage(Entity *, int);
static void dropReflectionArtifact(void);
static void flameWait(void);
static void flameReactToBlock(Entity *);
static void reflectionShieldInit(int);
static void reflectionShieldWait(void);
static void reflectionShieldTouch(Entity *);
static void shieldFlameWait(void);
static void swordDropInit(void);
static void shieldBiteInit(void);
static void reflectionShieldInit(int);
static void reflectionShieldTouch(Entity *);
static void shieldFlameAttackInit(void);
static void addShield(void);
static void addSword(void);
static void shieldDie(void);
static void swordWait(void);

Entity *addGrimlore(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Grimlore");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->takeDamage = &takeDamage;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

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

			self->thinkTime = 60;

			self->mental = 2;

			self->action = &doIntro;

			setContinuePoint(FALSE, self->name, NULL);
		}
	}

	checkToMap(self);
}

static void doIntro()
{
	addSword();
	addShield();

	self->action = &entityWait;
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = swordStabInit;
	}

	checkToMap(self);
}

static void attackFinished()
{
	self->thinkTime = 120;

	self->action = &entityWait;
}

static void swordStabInit()
{
	self->thinkTime = 30;

	self->maxThinkTime = 1;

	self->action = &swordStab;

	checkToMap(self);
}

static void addShield()
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Grimlore's Shield");
	}

	loadProperties("boss/grimlore_shield", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &shieldWait;

	e->draw = &drawLoopingAnimationToMap;
	e->takeDamage = &shieldTakeDamage;
	e->die = &shieldDie;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	e->head = self;
}

static void shieldDie()
{
	self->inUse = FALSE;
}

static void addSword()
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Grimlore's Sword");
	}

	loadProperties("boss/grimlore_sword", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &swordWait;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	e->head = self;
}

static void swordWait()
{
	if (self->maxThinkTime == 1)
	{
		self->action = &swordDropInit;
	}
}

static void swordStab()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Grimlore's Sword Stab");
		}

		loadProperties("boss/grimlore_sword_stab", e);

		e->x = self->x;
		e->y = self->y + self->h;

		e->action = &swordMoveUnderPlayer;

		e->draw = &drawLoopingAnimationToMap;
		e->takeDamage = &swordTakeDamage;
		e->die = &swordDie;

		e->type = ENEMY;

		setEntityAnimation(e, "STAND");

		e->startY = e->y;

		e->y -= e->h;

		e->endY = e->y;

		e->mental = 3 + prand() % 5;

		e->dirX = e->speed * 1.5;

		e->head = self;

		self->action = &swordStabWait;
	}

	checkToMap(self);
}

static void swordStabWait()
{
	if (self->maxThinkTime <= 0)
	{
		self->action = &attackFinished;
	}
}

static void swordMoveUnderPlayer()
{
	float target = player.x - self->w / 2 + player.w / 2;

	if (fabs(target - self->x) <= fabs(self->dirX))
	{
		self->targetY = self->y - self->h;

		self->thinkTime = 30;

		self->action = &swordRise;
	}

	else
	{
		self->x += target > self->x ? self->dirX : -self->dirX;

		if (self->x < self->startX)
		{
			self->x = self->startX;

			self->targetY = self->y - self->h;

			self->thinkTime = 30;

			self->action = &swordRise;
		}

		else if (self->x > self->endX)
		{
			self->x = self->endX;

			self->targetY = self->y - self->h;

			self->thinkTime = 30;

			self->action = &swordRise;
		}
	}
}

static void swordRise()
{
	Entity *e;

	if (self->y > self->startY)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->thinkTime == 0)
			{
				playSoundToMap("sound/common/crumble.ogg", BOSS_CHANNEL, self->x, self->y, 0);

				shakeScreen(MEDIUM, 15);
			}

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

			self->y -= 12;

			if (self->y <= self->startY)
			{
				self->y = self->startY;

				self->thinkTime = 30;
			}
		}
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime < 0)
		{
			self->action = &swordSink;
		}
	}

	facePlayer();
}

static void swordSink()
{
	if (self->y < self->targetY)
	{
		self->y += 3;
	}

	else
	{
		self->y = self->targetY;

		if (self->mental > 0 && player.health > 0)
		{
			self->action = &swordMoveUnderPlayer;

			self->dirX = self->speed * 1.5;
		}

		else
		{
			self->action = &swordFinish;

			self->thinkTime = 60;
		}
	}
}

static void swordFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->head->maxThinkTime = 0;

		self->inUse = FALSE;
	}
}

static void swordTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

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

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		addDamageScore(damage, self);
	}
}

static void swordDie()
{
	dropReflectionArtifact();

	self->head->maxThinkTime = 0;

	self->inUse = FALSE;
}

static void swordDropInit()
{
	Target *t;

	self->flags |= NO_DRAW;

	t = getTargetByName("GRIMLORE_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Grimlore cannot find target");
	}

	self->thinkTime = 60;

	self->flags |= FLY;

	self->action = &swordDrop;
}

static void swordDrop()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->flags &= ~(FLY|NO_DRAW);
		}
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->thinkTime = 120;

		self->action = &swordDropFinish;
	}
}

static void swordDropFinish()
{
	int d1, d2;
	Target *t1, *t2;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= NO_DRAW;

		t1 = getTargetByName("GRIMLORE_LEFT_TARGET");

		t2 = getTargetByName("GRIMLORE_RIGHT_TARGET");

		if (t1 == NULL || t2 == NULL)
		{
			showErrorAndExit("Grimlore cannot find target");
		}

		d1 = abs(player.x - t1->x);
		d2 = abs(player.x - t2->x);

		self->targetX = d1 < d2 ? t2->x : t1->x;
	}

	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		if (self->head->inUse == TRUE)
		{
			/* The shield will take the damage instead */

			shieldTakeDamage(other, damage);

			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
		}

		else if (self->mental > 0)
		{
			/* The armour will take the damage instead */

			armourTakeDamage(other, damage);

			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
		}

		else
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

			if (other->type == PROJECTILE)
			{
				temp = self;

				self = other;

				self->die();

				self = temp;
			}

			addDamageScore(damage, self);
		}
	}
}

static void armourTakeDamage(Entity *other, int damage)
{
	Entity *e, *temp;

	for (e=self->target;e!=NULL;e=e->target)
	{
		if (e->health <= 0)
		{
			continue;
		}

		if (!(e->flags & INVULNERABLE))
		{
			e->health -= damage;

			if (e->health <= 0)
			{
				e->thinkTime = 180;

				e->flags &= ~FLY;

				e->dirY = ITEM_JUMP_HEIGHT;

				e->action = e->die;

				self->mental--;
			}

			else
			{
				setCustomAction(e, &flashWhite, 6, 0, 0);

				setCustomAction(e, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

				enemyPain();
			}

			if (other->type == PROJECTILE)
			{
				temp = self;

				self = other;

				self->die();

				self = temp;
			}

			addDamageScore(damage, self);
		}

		return;
	}
}

static void shieldTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		self->health -= damage;

		if (self->health <= 0)
		{
			self->action = &shieldDie;
		}

		else
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

			enemyPain();
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		addDamageScore(damage, self);
	}

	return;
}

static void shieldWait()
{
	self->action = &shieldBiteInit;
	self->action = &shieldFlameAttackInit;
}

static void shieldBiteInit()
{
	self->startX = self->x;

	self->thinkTime = 60;

	self->action = &shieldBiteWait;
}

static void shieldBiteWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirX = self->face == LEFT ? -self->speed * 3 : self->speed * 3;

		self->action = &shieldBite;

		self->reactToBlock = &shieldBiteReactToBlock;
	}
}

static void shieldBiteReactToBlock(Entity *other)
{
	self->dirX = 0;
}

static void shieldBite()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->mental--;

		self->action = &shieldBiteReturn;

		self->dirX = self->face == LEFT ? self->speed * 3 : -self->speed * 3;
	}
}

static void shieldBiteReturn()
{
	if ((self->face == LEFT && self->x >= self->startX) || (self->face == RIGHT && self->x <= self->startX))
	{
		self->mental--;

		if (self->mental <= 0)
		{
			self->action = &shieldAttackFinish;
		}

		else
		{
			self->action = &shieldBiteWait;
		}
	}
}

static void shieldAttackFinish()
{
	self->head->maxThinkTime = 0;

	self->action = &shieldWait;
}

static void shieldFlameAttackInit()
{
	self->action = &shieldFlameAttack;

	self->thinkTime = 60;
}

static void shieldFlameAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Grimlore's Shield Flame");
		}

		loadProperties("enemy/grimlore_shield_flame", e);

		setEntityAnimation(e, "STAND");

		e->dirX = self->face == LEFT ? -1 : 1;

		e->action = &flameWait;

		e->draw = &drawLoopingAnimationToMap;

		e->touch = &entityTouch;

		e->reactToBlock = &flameReactToBlock;

		e->thinkTime = 10;

		e->mental = 1;

		e->health = 0;

		e->head = self;

		e->x = self->x;

		e->startX = e->x;
	}

	self->action = &shieldFlameWait;

	self->maxThinkTime = 1;
}

static void shieldFlameWait()
{
	if (self->maxThinkTime <= 0)
	{
		self->action = &attackFinished;
	}
}

static void flameReactToBlock(Entity *other)
{
	self->x = self->startX;
}

static void flameWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->health += self->mental;

		if (self->health >= 8)
		{
			self->mental = -1;

			self->thinkTime = 60;
		}

		else
		{
			self->thinkTime = 10;
		}

		if (self->health < 0)
		{
			self->head->maxThinkTime = 0;

			self->inUse = FALSE;
		}

		else
		{
			setEntityAnimation(self, "STAND");
		}
	}
}

static void dropReflectionArtifact()
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Reflect Artifact");
	}

	loadProperties("boss/grimlore_reflect_artifact", e);

	e->x = self->x + self->w / 2;
	e->y = self->y + self->h / 2;

	e->action = &doNothing;

	e->activate = &reflectionShieldInit;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &keyItemTouch;

	e->dirY = ITEM_JUMP_HEIGHT;
}

static void reflectionShieldInit(int val)
{
	Entity *e;

	if (game.status == IN_GAME)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Reflection Shield");
		}

		loadProperties("boss/artifact_reflection_shield", e);

		e->action = &reflectionShieldWait;

		e->draw = &drawLoopingAnimationToMap;

		e->touch = &reflectionShieldTouch;
	}
}

static void reflectionShieldWait()
{
	if (player.dirX != 0)
	{
		self->inUse = FALSE;
	}
}

static void reflectionShieldTouch(Entity *other)
{

}
