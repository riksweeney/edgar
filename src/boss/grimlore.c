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
#include "../projectile.h"
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
static void swordStabMoveUnderPlayer(void);
static void swordStabRise(void);
static void swordStabSink(void);
static void swordStabFinish(void);
static void swordTakeDamage(Entity *, int);
static void swordStabTakeDamage(Entity *, int);
static void swordDie(void);
static void swordDropInit(void);
static void swordDrop(void);
static void swordDropTeleportAway(void);
static void swordDropFinish(void);
static void takeDamage(Entity *, int);
static void armourTakeDamage(Entity *, int);
static void shieldWait(void);
static void shieldBiteInit(void);
static void shieldBiteWait(void);
static void shieldBite(void);
static void shieldBiteReturn(void);
static void shieldAttackFinish(void);
static void shieldFlameAttack(void);
static void shieldTakeDamage(Entity *, int);
static void flameWait(void);
static void swordDropInit(void);
static void shieldFlameAttackInit(void);
static void addShield(void);
static void addSword(void);
static void addArmour(void);
static void armourWait(void);
static void armourDie(void);
static void shieldDie(void);
static void swordWait(void);
static void shieldAttackWait(void);
static void swordStabPause(void);
static void shieldFaceTakeDamage(Entity *, int);
static int biteDraw(void);
static void shieldBiteReactToBlock(Entity *);
static void shieldBiteMoveBack(void);
static void dropReflectionArtifact(void);

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
		if (cameraAtMinimum())
		{
			self->flags &= ~NO_DRAW;
			
			addArmour();

			addSword();

			addShield();

			centerMapOnEntity(NULL);

			self->thinkTime = 60;

			self->action = &doIntro;

			setContinuePoint(FALSE, self->name, NULL);

			playDefaultBossMusic();

			initBossHealthBar();

			self->flags |= LIMIT_TO_SCREEN;

			self->mental = 7;

			self->touch = &entityTouch;
		}
	}

	checkToMap(self);
}

static void doIntro()
{
	self->face = LEFT;

	self->action = &attackFinished;
}

static void entityWait()
{
	int r;

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		if (self->mental & 2) /* Has the Sword */
		{
			if (self->mental & 4) /* Has the Shield aswell */
			{
				r = prand() % 4;

				switch (r)
				{
					case 0:
						self->action = &swordStabInit;
					break;

					case 1:
						self->maxThinkTime = 2;
						self->action = &shieldAttackWait;
					break;

					case 2:
						self->maxThinkTime = 3;
						self->action = &shieldAttackWait;
					break;

					default:
						self->action = &swordDropInit;
					break;
				}
			}

			else /* Only the sword */
			{
				r = prand() % 2;

				switch (r)
				{
					case 0:
						self->action = &swordStabInit;
					break;

					default:
						self->action = &swordDropInit;
					break;
				}
			}
		}

		else if (self->mental & 4)
		{
			if (self->mental & 2) /* Has the Sword */
			{
				r = prand() % 4;

				switch (r)
				{
					case 0:
						self->action = &swordStabInit;
					break;

					case 1:
						self->maxThinkTime = 2;
						self->action = &shieldAttackWait;
					break;

					case 2:
						self->maxThinkTime = 3;
						self->action = &shieldAttackWait;
					break;

					default:
						self->action = &swordDropInit;
					break;
				}
			}

			else /* Only the Sword */
			{
				r = prand() % 2;

				switch (r)
				{
					case 0:
						self->action = &swordStabInit;
					break;

					default:
						self->action = &swordDropInit;
					break;
				}
			}
		}
	}

	checkToMap(self);
}

static void attackFinished()
{
	setEntityAnimation(self, "STAND");

	self->thinkTime = 120;

	self->action = &entityWait;
}

static void addArmour()
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Grimlore's Armour");
	}

	loadProperties("boss/grimlore_armour", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &armourWait;

	e->draw = &drawLoopingAnimationToMap;
	e->takeDamage = &armourTakeDamage;
	e->die = &armourDie;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	e->head = self;
}

static void armourWait()
{
	setEntityAnimation(self, self->head->animationName);
	
	self->face = self->head->face;

	if (self->head->flags & NO_DRAW)
	{
		self->flags |= NO_DRAW;
	}

	else
	{
		self->flags &= ~NO_DRAW;
	}

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;
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
	e->touch = &entityTouch;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	e->head = self;
}

static void shieldWait()
{
	setEntityAnimation(self, self->head->animationName);

	if (self->head->maxThinkTime == 2)
	{
		self->action = &shieldFlameAttackInit;
	}

	else if (self->head->maxThinkTime == 3)
	{
		self->action = &shieldBiteInit;
	}

	self->face = self->head->face;

	if (self->head->flags & NO_DRAW)
	{
		self->flags |= NO_DRAW;
	}

	else
	{
		self->flags &= ~NO_DRAW;
	}

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;
}

static void shieldDie()
{
	dropReflectionArtifact();

	if (self->head->maxThinkTime == 2 || self->head->maxThinkTime == 3)
	{
		self->head->maxThinkTime = 0;
	}

	printf("1. Shield dying. Grimlore mental is %d\n",self->head->mental);

	self->head->mental -= 4;

	printf("2. Shield dying. Grimlore mental is %d\n",self->head->mental);

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

	e->touch = &entityTouch;

	e->takeDamage = &swordTakeDamage;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	e->head = self;
}

static void swordWait()
{
	setEntityAnimation(self, self->head->animationName);

	if (self->head->maxThinkTime == 4)
	{
		self->action = &swordStab;
	}

	if (self->head->flags & NO_DRAW)
	{
		self->flags |= NO_DRAW;
	}

	else
	{
		self->flags &= ~NO_DRAW;
	}

	self->face = self->head->face;

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;
}

static void swordStabInit()
{
	setEntityAnimation(self, "KNEEL");

	self->thinkTime = 60;

	self->action = &swordStabPause;
}

static void swordStabPause()
{
	self->thinkTime--;

	/* Check the sword still exists during the stab motion */

	if (!(self->mental & 2))
	{
		self->action = &attackFinished;
	}

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "KNEEL_STAB");

		self->maxThinkTime = 4;

		playSoundToMap("sound/enemy/ground_spear/spear.ogg", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &shieldAttackWait;
	}
}

static void swordStab()
{
	Entity *e;

	self->thinkTime--;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Grimlore's Sword Stab");
	}

	loadProperties("boss/grimlore_sword_stab", e);

	setEntityAnimation(e, "STAND");

	e->x = self->x;
	e->y = self->y;

	e->startX = getMapStartX();
	e->endX = e->startX + SCREEN_WIDTH - e->w - 1;

	e->action = &swordStabMoveUnderPlayer;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->takeDamage = &swordStabTakeDamage;
	e->die = &swordDie;

	e->type = ENEMY;

	e->y = self->y - e->h + self->h;

	e->startY = e->y;

	e->y += e->h;

	e->endY = e->y;

	e->mental = 3 + prand() % 5;

	e->dirX = e->speed * 1.5;

	e->head = self;

	e->health = self->health;

	self->maxThinkTime = 1;

	self->action = &swordStabWait;

	checkToMap(self);
}

static void swordStabWait()
{
	if (self->maxThinkTime <= 0)
	{
		self->head->maxThinkTime = 0;

		self->action = &swordWait;
	}
}

static void swordStabMoveUnderPlayer()
{
	float target = player.x - self->w / 2 + player.w / 2;

	if (self->head->health <= 0)
	{
		self->inUse = FALSE;
	}

	if (fabs(target - self->x) <= fabs(self->dirX))
	{
		self->targetY = self->y - self->h;

		self->thinkTime = 30;

		self->action = &swordStabRise;
	}

	else
	{
		self->x += target > self->x ? self->dirX : -self->dirX;

		if (self->x < self->startX)
		{
			self->x = self->startX;

			self->targetY = self->y - self->h;

			self->thinkTime = 30;

			self->action = &swordStabRise;
		}

		else if (self->x > self->endX)
		{
			self->x = self->endX;

			self->targetY = self->y - self->h;

			self->thinkTime = 30;

			self->action = &swordStabRise;
		}
	}
}

static void swordStabRise()
{
	Entity *e;

	if (self->head->health <= 0)
	{
		self->inUse = FALSE;
	}

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
			self->action = &swordStabSink;
		}
	}
}

static void swordStabSink()
{
	if (self->head->health <= 0)
	{
		self->inUse = FALSE;
	}

	if (self->y < self->endY)
	{
		self->y += 3;
	}

	else
	{
		self->y = self->endY;

		self->mental--;

		if (self->mental > 0 && player.health > 0)
		{
			self->action = &swordStabMoveUnderPlayer;

			self->dirX = self->speed * 1.5;
		}

		else
		{
			self->action = &swordStabFinish;

			self->thinkTime = 60;
		}
	}
}

static void swordStabFinish()
{
	self->thinkTime--;

	if (self->head->health <= 0)
	{
		self->inUse = FALSE;
	}

	if (self->thinkTime <= 0)
	{
		self->head->maxThinkTime = 0;

		self->inUse = FALSE;
	}
}

static void swordStabTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->head->health -= damage;

		if (self->head->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

			enemyPain();
		}

		else
		{
			self->damage = 0;

			self->touch = NULL;

			self->action = &entityDieNoDrop;

			self->head->touch = NULL;

			self->head->action = &swordDie;
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

static void swordTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if ((self->flags & INVULNERABLE) || (self->head->mental & 4))
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

			enemyPain();
		}

		else
		{
			self->damage = 0;

			self->touch = NULL;

			self->action = &swordDie;
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

	if (self->head->maxThinkTime == 4)
	{
		self->head->maxThinkTime = 0;
	}

	printf("1. Sword dying. Grimlore mental is %d\n",self->head->mental);

	self->head->mental -= 2;

	printf("2. Sword dying. Grimlore mental is %d\n",self->head->mental);

	self->inUse = FALSE;
}

static void armourDie()
{
	dropReflectionArtifact();

	printf("1. Armour dying. Grimlore mental is %d\n",self->head->mental);

	self->head->mental -= 1;

	printf("2. Armour dying. Grimlore mental is %d\n",self->head->mental);

	self->inUse = FALSE;
}

static void swordDropInit()
{
	Target *t;

	self->flags |= NO_DRAW;

	setEntityAnimation(self, "KNEEL");

	playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	t = getTargetByName("GRIMLORE_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Grimlore cannot find target");
	}

	self->y = t->y;

	self->thinkTime = 60;

	self->flags |= FLY;

	self->action = &swordDrop;
}

static void swordDrop()
{
	int i;

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		self->x = player.x + player.w / 2 - self->w / 2;

		if (self->thinkTime <= 0)
		{
			playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

			self->flags &= ~(FLY|NO_DRAW);

			self->dirY = 0;
		}
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		setEntityAnimation(self, "KNEEL_STAB");

		playSoundToMap("sound/enemy/red_grub/thud.ogg", BOSS_CHANNEL, self->x, self->y, 0);

		shakeScreen(LIGHT, 15);

		for (i=0;i<20;i++)
		{
			addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
		}

		self->thinkTime = 120;

		self->action = &swordDropTeleportAway;
	}
}

static void swordDropTeleportAway()
{
	int d1, d2;
	Target *t1, *t2;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 30;

		playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

		setCustomAction(self, &invulnerableNoFlash, self->thinkTime, 0, 0);

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		self->flags |= NO_DRAW;

		t1 = getTargetByName("GRIMLORE_LEFT_TARGET");

		t2 = getTargetByName("GRIMLORE_RIGHT_TARGET");

		if (t1 == NULL || t2 == NULL)
		{
			showErrorAndExit("Grimlore cannot find target");
		}

		d1 = abs(player.x - t1->x);
		d2 = abs(player.x - t2->x);

		if (d1 < d2)
		{
			self->targetX = t2->x;

			self->face = LEFT;
		}

		else
		{
			self->targetX = t1->x;

			self->face = RIGHT;
		}

		self->action = &swordDropFinish;
	}

	checkToMap(self);
}

static void swordDropFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->x = self->targetX;

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		self->flags &= ~NO_DRAW;

		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	/*
	Armour sets mental to 1
	Sword sets mental to 2
	Shield sets mental to 4
	*/

	if (!(self->flags & INVULNERABLE))
	{
		if (self->mental & 4)
		{
			/* The shield will take the damage instead */

			return;
		}

		else if (self->mental & 2)
		{
			/* The sword will take the damage instead */

			return;
		}

		else if (self->mental & 1)
		{
			/* The armour will take the damage instead */

			return;
		}

		else
		{
			/* Take minimal damage from bombs */

			if (other->type == EXPLOSION)
			{
				damage = 1;
			}

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
	Entity *temp;
	
	if ((self->head->mental & 4) || (self->head->mental & 2))
	{
		return;
	}

	if (!(self->flags & INVULNERABLE))
	{
		self->health -= damage;

		if (self->health <= 0)
		{
			self->touch = NULL;

			self->action = &armourDie;
		}

		else
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

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

static void shieldTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		self->health -= damage;

		if (self->health <= 0)
		{
			self->touch = NULL;

			self->action = &shieldDie;
		}

		else
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

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

static void shieldFaceTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		self->health -= damage;

		if (self->health <= 0)
		{
			self->action = &entityDieNoDrop;

			self->head->touch = NULL;

			self->head->action = &shieldDie;
		}

		else
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

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

static void shieldAttackWait()
{
	if (self->maxThinkTime <= 0)
	{
		self->action = &attackFinished;
	}
}

static void shieldBiteInit()
{
	self->thinkTime = 60;

	setEntityAnimation(self, "FLAME_ATTACK");

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;

	self->action = &shieldBiteWait;
}

static void shieldBiteWait()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Grimlore's Shield Bite");
		}

		loadProperties("boss/grimlore_shield_bite", e);

		setEntityAnimation(e, "STAND");

		e->action = &shieldBite;

		e->draw = &biteDraw;

		e->touch = &entityTouch;

		e->takeDamage = &shieldFaceTakeDamage;

		e->reactToBlock = &shieldBiteReactToBlock;

		e->thinkTime = 180;

		e->health = self->health;

		e->head = self;

		e->face = self->face;

		if (e->face == LEFT)
		{
			e->x = self->x + self->w - e->w - e->offsetX;
		}

		else
		{
			e->x = self->x + e->offsetX;
		}

		e->y = self->y + e->offsetY;

		e->startX = e->x;

		e->startY = e->y;

		e->thinkTime = 60;

		e->mental = 3;

		self->maxThinkTime = 1;

		setEntityAnimation(self, "NO_FACE");

		self->action = &shieldAttackFinish;
	}
}

static void shieldBiteReactToBlock(Entity *other)
{
	self->endX = self->face == LEFT ? self->x + 64 : self->x - 64;

	self->dirX = self->face == LEFT ? 2 : -2;

	self->action = &shieldBiteMoveBack;
}

static void shieldBiteMoveBack()
{
	if ((self->face == LEFT && self->x >= self->endX) || (self->face == RIGHT && self->x <= self->endX))
	{
		self->mental--;

		self->dirX = 0;

		if (self->mental <= 0)
		{
			self->thinkTime = 60;

			self->flags |= UNBLOCKABLE;
		}

		else
		{
			self->thinkTime = 1;
		}

		self->action = &shieldBite;
	}

	checkToMap(self);
}

static void shieldBite()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		self->y = self->startY + sin(DEG_TO_RAD(self->endY)) * 2;

		self->endY += 90;

		if (self->endY >= 360)
		{
			self->endY = 0;
		}

		if (self->thinkTime <= 0)
		{
			self->y = self->startY;

			self->dirX = self->face == LEFT ? -16 : 16;

			self->flags |= LIMIT_TO_SCREEN;
		}
	}

	else
	{
		if (self->dirX == 0)
		{
			self->thinkTime = self->mental <= 0 ? 120 : 1;

			self->flags |= UNBLOCKABLE;

			self->action = &shieldBiteReturn;
		}
	}

	checkToMap(self);
}

static void shieldBiteReturn()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 60)
		{
			self->y = self->startY + sin(DEG_TO_RAD(self->endY)) * 2;

			self->endY += 90;

			if (self->endY >= 360)
			{
				self->endY = 0;
			}
		}

		if (self->thinkTime <= 0)
		{
			self->y = self->startY;

			self->dirX = self->face == LEFT ? 16 : -16;
		}
	}

	if ((self->face == LEFT && self->x >= self->startX) || (self->face == RIGHT && self->x <= self->startX))
	{
		self->head->maxThinkTime = 0;

		self->inUse = FALSE;
	}

	checkToMap(self);
}

static void shieldAttackFinish()
{
	if (self->maxThinkTime <= 0)
	{
		self->head->maxThinkTime = 0;

		self->action = &shieldWait;
	}
}

static void shieldFlameAttackInit()
{
	setEntityAnimation(self, "FLAME_ATTACK");

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;

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

		loadProperties("enemy/fireball", e);

		setEntityAnimation(e, "STAND");

		e->face = self->face;

		e->action = &flameWait;

		e->draw = &drawLoopingAnimationToMap;

		e->thinkTime = 300;

		e->mental = 1;

		e->health = 0;

		e->head = self;

		e->x = self->x + (self->face == LEFT ? -e->w : e->w);

		e->y = self->y + self->h / 2 - e->h / 2;

		self->action = &shieldAttackFinish;

		self->maxThinkTime = 1;
	}
}

static void flameWait()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->head->maxThinkTime = 0;

		self->inUse = FALSE;
	}

	else if (self->thinkTime % 6 == 0)
	{
		e = addProjectile("enemy/fireball", self, 0, 0, (self->face == LEFT ? -6 : 6), 0);

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		e->flags |= FLY;
	}
	/*
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
	*/
}

static void dropReflectionArtifact()
{
}

static int biteDraw()
{
	int startX;

	startX = self->x;

	/* Draw the segments first */

	setEntityAnimation(self, "CHAIN");

	if (self->face == RIGHT)
	{
		self->x -= self->w;

		while (self->x >= self->startX)
		{
			drawSpriteToMap();

			self->x -= self->w;
		}
	}

	else
	{
		self->x += self->w;

		while (self->x <= self->startX)
		{
			drawSpriteToMap();

			self->x += self->w;
		}
	}

	/* Draw the tip */

	setEntityAnimation(self, "STAND");

	self->x = startX;

	drawLoopingAnimationToMap();

	return TRUE;
}
