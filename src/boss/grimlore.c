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
#include "../boss/grimlore_summon_spell.h"
#include "../collisions.h"
#include "../credits.h"
#include "../custom_actions.h"
#include "../enemy/magic_missile.h"
#include "../enemy/rock.h"
#include "../entity.h"
#include "../game.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/graphics.h"
#include "../hud.h"
#include "../inventory.h"
#include "../item/grimlore_artifact.h"
#include "../item/item.h"
#include "../map.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/explosion.h"
#include "../world/target.h"

extern Entity *self, player;
extern Game game;

static void initialise(void);
static void entityWait(void);
static void doIntro(void);
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
static void swordStabDie(void);
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
static void dropProtectionArtifact(void);
static void dropBindArtifact(void);
static void magicMissileAttackInit(void);
static void magicMissileAttack(void);
static void beamAttackInit(void);
static void beamAttack(void);
static void beamMove(void);
static int beamDraw(void);
static void beamExplosions(void);
static void explosionTouch(Entity *);
static void magicMissileChargeWait(void);
static void itemDestroyAttackInit(void);
static void itemDestroyAttack(void);
static void rotateAroundPlayer(void);
static void itemDestroyWait(void);
static void itemDestroyerTakeDamage(Entity *, int);
static void itemDestroyerExpand(void);
static void itemDestroyerRetract(void);
static void destroyInventoryItem(void);
static void crushAttackInit(void);
static void crushAttackAppear(void);
static void crushAttackWait(void);
static void fistMoveAbovePlayer(void);
static void fistCrush(void);
static void crushWait(void);
static void fistVanish(void);
static void stunInit(void);
static void stunWait(void);
static void fistTakeDamage(Entity *, int);
static void crushToMiddleAttackInit(void);
static void crushToMiddleAttackAppear(void);
static void fistMoveToMiddle(void);
static void fistVanishWait(void);
static void fistAppearActivate(void);
static int flameDraw(void);
static void resetWeapons(void);
static void bindInit(void);
static void bindWait(void);
static void creditsMove(void);

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

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "FACE_FRONT");

	return e;
}

static void initialise()
{
	if (self->active == FALSE)
	{
		addArmour();

		addSword();

		addShield();

		self->active = TRUE;

		self->thinkTime = 60;
	}

	else
	{
		self->face = LEFT;

		if (self->head == NULL)
		{
			self->startY = self->y;

			centerMapOnEntity(NULL);

			self->action = &doIntro;

			self->active = FALSE;

			self->flags |= LIMIT_TO_SCREEN;
		}
	}

	checkToMap(self);
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->touch = &entityTouch;

		playDefaultBossMusic();

		initBossHealthBar();

		self->action = &attackFinished;

		self->endY = 0;
	}

	checkToMap(self);
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
						self->maxThinkTime = 5;
						self->action = &swordStabInit;
					break;

					case 1:
						setEntityAnimation(self, "SHIELD_ATTACK");
						self->maxThinkTime = 2;
						self->action = &shieldAttackWait;
					break;

					case 2:
						setEntityAnimation(self, "SHIELD_ATTACK");
						self->maxThinkTime = 3;
						self->action = &shieldAttackWait;
					break;

					default:
						self->maxThinkTime = 5;
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
						self->maxThinkTime = 5;
						self->action = &swordStabInit;
					break;

					default:
						self->maxThinkTime = 5;
						self->action = &swordDropInit;
					break;
				}
			}
		}

		else if (self->mental & 4) /* Has the Shield */
		{
			if (self->mental & 2) /* Has the Sword aswell */
			{
				r = prand() % 4;

				switch (r)
				{
					case 0:
						self->maxThinkTime = 5;
						self->action = &swordStabInit;
					break;

					case 1:
						setEntityAnimation(self, "SHIELD_ATTACK");
						self->maxThinkTime = 2;
						self->action = &shieldAttackWait;
					break;

					case 2:
						setEntityAnimation(self, "SHIELD_ATTACK");
						self->maxThinkTime = 3;
						self->action = &shieldAttackWait;
					break;

					default:
						self->maxThinkTime = 5;
						self->action = &swordDropInit;
					break;
				}
			}

			else /* Only the Shield */
			{
				r = prand() % 2;

				switch (r)
				{
					case 0:
						setEntityAnimation(self, "SHIELD_ATTACK");
						self->maxThinkTime = 2;
						self->action = &shieldAttackWait;
					break;

					default:
						setEntityAnimation(self, "SHIELD_ATTACK");
						self->maxThinkTime = 3;
						self->action = &shieldAttackWait;
					break;
				}
			}
		}

		else /* No weapons */
		{
			r = prand() % 5;

			switch (r)
			{
				case 0:
					self->action = &magicMissileAttackInit;
				break;

				case 1:
					self->action = &itemDestroyAttackInit;
				break;

				case 2:
					self->action = &crushAttackInit;
				break;

				case 3:
					self->action = &crushToMiddleAttackInit;
				break;

				default:
					self->action = &beamAttackInit;
				break;
			}
		}
	}

	checkToMap(self);
}

static void attackFinished()
{
	self->maxThinkTime = 0;

	setEntityAnimation(self, "STAND");

	self->flags &= ~FLY;

	self->thinkTime = 60;

	self->action = &entityWait;

	self->layer = MID_GROUND_LAYER;

	if (self->endY == -99)
	{
		setEntityAnimation(self, "RAISE_ARMS_1");

		self->action = &resetWeapons;
	}
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
	e->die = &armourDie;
	e->touch = &entityTouch;

	e->creditsAction = &armourWait;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	e->head = self;

	self->mental += 1;
}

static void armourWait()
{
	if (self->head->head == NULL)
	{
		self->head->head = self;
	}

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

	self->x = self->head->x;

	self->y = self->head->y;

	if (self->health <= 0 && !(self->flags & NO_DRAW))
	{
		self->die();
	}

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
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

	e->creditsAction = &shieldWait;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	e->head = self;

	self->mental += 2;
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

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
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

	e->creditsAction = &swordWait;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	e->head = self;

	self->mental += 4;
}

static void swordWait()
{
	self->layer = self->head->maxThinkTime >= 4 || self->head->touch == NULL ? MID_GROUND_LAYER : BACKGROUND_LAYER;

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

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static void swordStabInit()
{
	setEntityAnimation(self, "KNEEL");

	self->layer = BACKGROUND_LAYER;

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

		playSoundToMap("sound/enemy/ground_spear/spear", BOSS_CHANNEL, self->x, self->y, 0);

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
	e->die = &swordStabDie;

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
		self->die();

		return;
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
		self->die();

		return;
	}

	if (self->y > self->startY)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->thinkTime == 0)
			{
				playSoundToMap("sound/common/crumble", BOSS_CHANNEL, self->x, self->y, 0);

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
		self->die();

		return;
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

			self->takeDamage = NULL;

			self->head->takeDamage = NULL;

			self->head->action = &swordDie;

			self->die();
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}
	}
}

static void swordTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->head->touch == NULL)
	{
		return;
	}

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
	}
}

static void swordDie()
{
	int i;
	Entity *e;

	dropReflectionArtifact();

	if (self->head->maxThinkTime == 4)
	{
		self->head->maxThinkTime = 0;
	}

	self->head->mental -= 2;

	for (i=0;i<7;i++)
	{
		e = addTemporaryItem("boss/grimlore_sword_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += self->w / 2 - e->w / 2;
		e->y += self->h / 2 - e->h / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->thinkTime = 60 + (prand() % 60);
	}

	playSoundToMap("sound/common/shatter", BOSS_CHANNEL, player.x, player.y, 0);

	self->inUse = FALSE;
}

static void swordStabDie()
{
	int i;
	Entity *e;

	if (self->head->maxThinkTime == 4)
	{
		self->head->maxThinkTime = 0;
	}

	self->head->mental -= 2;

	for (i=0;i<7;i++)
	{
		e = addTemporaryItem("boss/grimlore_sword_stab_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += self->w / 2 - e->w / 2;
		e->y += self->h / 2 - e->h / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->thinkTime = 60 + (prand() % 60);
	}

	self->inUse = FALSE;
}

static void armourDie()
{
	int i;
	Entity *e;

	dropBindArtifact();

	self->head->head = NULL;

	self->head->mental -= 1;

	for (i=0;i<7;i++)
	{
		e = addTemporaryItem("boss/grimlore_armour_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += self->w / 2 - e->w / 2;
		e->y += self->h / 2 - e->h / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->thinkTime = 60 + (prand() % 60);
	}

	playSoundToMap("sound/common/shatter", BOSS_CHANNEL, player.x, player.y, 0);

	self->inUse = FALSE;
}

static void shieldDie()
{
	int i;
	Entity *e;

	dropProtectionArtifact();

	if (self->head->maxThinkTime == 2 || self->head->maxThinkTime == 3)
	{
		self->head->maxThinkTime = 0;
	}

	self->head->mental -= 4;

	for (i=0;i<7;i++)
	{
		e = addTemporaryItem("boss/grimlore_shield_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += self->w / 2 - e->w / 2;
		e->y += self->h / 2 - e->h / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->thinkTime = 60 + (prand() % 60);
	}

	playSoundToMap("sound/common/shatter", BOSS_CHANNEL, player.x, player.y, 0);

	self->inUse = FALSE;
}

static void swordDropInit()
{
	Target *t;

	self->flags |= NO_DRAW;

	self->layer = BACKGROUND_LAYER;

	setEntityAnimation(self, "KNEEL");

	playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

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
		self->flags |= INVULNERABLE;

		self->thinkTime--;

		self->x = player.x + player.w / 2 - self->w / 2;

		if (self->thinkTime <= 0)
		{
			playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

			self->flags &= ~(FLY|NO_DRAW);

			self->dirY = 0;
		}
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		setEntityAnimation(self, "KNEEL_STAB");

		playSoundToMap("sound/enemy/red_grub/thud", BOSS_CHANNEL, self->x, self->y, 0);

		shakeScreen(LIGHT, 15);

		for (i=0;i<20;i++)
		{
			addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
		}

		self->thinkTime = 30;

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

		if (!(self->flags & NO_DRAW))
		{
			playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

			setCustomAction(self, &invulnerableNoFlash, self->thinkTime, 0, 0);

			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);
		}

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
			self->x = t2->x;

			self->face = LEFT;
		}

		else
		{
			self->x = t1->x;

			self->face = RIGHT;
		}

		self->y = self->startY;

		self->action = &swordDropFinish;
	}

	checkToMap(self);
}

static void swordDropFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		self->flags &= ~NO_DRAW;

		self->action = self->health <= 0 ? &stunInit : &attackFinished;
	}

	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{
	int health;
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

			temp = self;

			self = self->head;

			armourTakeDamage(self, damage);

			self = temp;

			return;
		}

		else
		{
			/* Take minimal damage from bombs */

			if (other->type == EXPLOSION)
			{
				damage = 1;
			}

			health = self->health;

			self->health -= damage;

			if (self->health <= 0 && health > 0)
			{
				self->health = 0;
			}

			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

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
	}

	return;
}

static void shieldTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->head->touch == NULL)
	{
		return;
	}

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

	setEntityAnimation(self, "SHIELD_ATTACK");

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

	self->action = &shieldBiteMoveBack;
}

static void shieldBiteMoveBack()
{
	self->dirX = self->face == LEFT ? 2 : -2;

	if ((self->face == LEFT && (self->x >= self->endX || self->x >= self->startX)) || (self->face == RIGHT && (self->x <= self->endX || self->x <= self->startX)))
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
	if (self->head->health <= 0)
	{
		self->action = &entityDieNoDrop;

		return;
	}

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
	if (self->head->health <= 0)
	{
		self->action = &entityDieNoDrop;

		return;
	}

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
		setEntityAnimation(self, "SHIELD_ATTACK_FLAME");

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->head->maxThinkTime = 0;

			self->action = &shieldWait;
		}
	}
}

static void shieldFlameAttackInit()
{
	setEntityAnimation(self, "SHIELD_ATTACK_FLAME");

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
		setEntityAnimation(self, "SHIELD_ATTACK");

		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Grimlore's Shield Flame");
		}

		loadProperties("boss/grimlore_shield_flame", e);

		setEntityAnimation(e, "HEAD");

		e->face = self->face;

		e->action = &flameWait;

		e->draw = &flameDraw;

		e->thinkTime = 180;

		e->mental = 1;

		e->health = 1200;

		e->head = self;

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - e->w - e->offsetX;
		}

		else
		{
			e->x = self->x + e->offsetX;
		}

		e->y = self->y + e->offsetY;

		e->dirX = 14;

		e->startX = e->x;

		e->endX = e->face == LEFT ? getMapStartX() - SCREEN_WIDTH : getMapStartX() + SCREEN_WIDTH;

		e->endY = playSoundToMap("sound/enemy/fire_burner/flame", BOSS_CHANNEL, self->x, self->y, -1);

		self->thinkTime = 60;

		self->action = &shieldAttackFinish;

		self->maxThinkTime = 1;
	}
}

static int flameDraw()
{
	int frame;
	float timer;

	self->x = self->startX;

	drawLoopingAnimationToMap();

	frame = self->currentFrame;
	timer = self->frameTimer;

	if (self->face == LEFT)
	{
		self->x -= self->w;

		setEntityAnimation(self, "BODY");

		self->currentFrame = frame;
		self->frameTimer = timer;

		while (self->x >= self->endX)
		{
			drawSpriteToMap();

			self->x -= self->w;
		}
	}

	else
	{
		self->x += self->w;

		setEntityAnimation(self, "BODY");

		self->currentFrame = frame;
		self->frameTimer = timer;

		while (self->x <= self->endX)
		{
			drawSpriteToMap();

			self->x += self->w;
		}
	}

	setEntityAnimation(self, "HEAD");

	self->currentFrame = frame;
	self->frameTimer = timer;

	return TRUE;
}

static void flameWait()
{
	int startX;
	Entity *e;

	self->health--;

	if (self->health % 10 == 0)
	{
		startX = self->face == LEFT ? self->endX : self->startX;

		if (collision(player.x, player.y, player.w, player.h, startX, self->y, abs(self->startX - self->endX), self->h) == 1)
		{
			e = addProjectile("enemy/fireball", self->head, 0, 0, (self->face == LEFT ? -self->dirX : self->dirX), 0);

			e->damage = 1;

			e->x = player.x + player.w / 2 - e->w / 2;
			e->y = player.y + player.h / 2 - e->h / 2;

			e->flags |= FLY|NO_DRAW;
		}
	}

	self->thinkTime--;

	if (self->thinkTime <= 0 || self->head->health <= 0)
	{
		stopSound(self->endY);

		self->head->maxThinkTime = 0;

		self->inUse = FALSE;
	}
}

static void dropReflectionArtifact()
{
	Entity *e = addReflectionArtifact(self->x, self->y, "item/reflection_artifact");

	e->dirY = ITEM_JUMP_HEIGHT;

	e->dirX = self->face == LEFT ? -6 : 6;
}

static void dropProtectionArtifact()
{
	Entity *e = addProtectionArtifact(self->x, self->y, "item/protection_artifact");

	e->dirY = ITEM_JUMP_HEIGHT;

	e->dirX = self->face == LEFT ? -6 : 6;
}

static void dropBindArtifact()
{
	Entity *e = addBindArtifact(self->x, self->y, "item/bind_artifact");

	e->dirY = ITEM_JUMP_HEIGHT;

	e->dirX = self->face == LEFT ? -6 : 6;
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

static void magicMissileAttackInit()
{
	int i;
	Entity *e;

	setEntityAnimation(self, "MAGIC_MISSILE_START");

	self->thinkTime = 60;

	self->endX = 4;

	self->action = &magicMissileAttack;

	self->thinkTime = 0;

	for (i=0;i<12;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Magic Missile particle");
		}

		loadProperties("boss/grimlore_magic_missile_particle", e);

		setEntityAnimation(e, i % 2 == 0 ? "LEFT" : "RIGHT");

		e->head = self;

		if (self->face == LEFT)
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

		e->draw = &drawLoopingAnimationToMap;

		e->mental = 180;

		e->health = i * 60 + (i % 2 == 0 ? 0 : 30);

		e->action = &magicMissileChargeWait;

		self->thinkTime++;
	}

	checkToMap(self);
}

static void magicMissileAttack()
{
	Entity *e;

	if (self->endX < 4)
	{
		self->thinkTime--;
	}

	if (self->thinkTime <= 0)
	{
		self->endX--;

		setEntityAnimation(self, (int)self->endX % 2 == 0 ? "MAGIC_MISSILE_LEFT" : "MAGIC_MISSILE_RIGHT");

		e = addMagicMissile(self->x, self->y, "enemy/magic_missile");

		playSoundToMap("sound/boss/awesome_boss/hadouken", BOSS_CHANNEL, self->x, self->y, 0);

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - e->w - self->offsetX;
		}

		else
		{
			e->x = self->x + self->offsetX;
		}

		e->y = self->y + self->offsetY;

		e->dirX = self->face == LEFT ? -e->speed : e->speed;
		e->dirY = 0;
		e->thinkTime = 90;

		e->parent = self;

		STRNCPY(e->requires, self->objectiveName, sizeof(e->requires));

		if (self->endX <= 0)
		{
			self->action = self->health <= 0 ? &swordDropTeleportAway : &attackFinished;
		}

		else
		{
			self->thinkTime = 20;
		}
	}

	checkToMap(self);
}

static void beamAttackInit()
{
	Target *t;

	self->flags |= NO_DRAW;

	setEntityAnimation(self, "BEAM_ATTACK");

	playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	t = getTargetByName("GRIMLORE_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Grimlore cannot find target");
	}

	self->y = t->y;

	self->thinkTime = 60;

	self->flags |= FLY;

	self->action = &beamAttack;

	self->endX = 1;
}

static void beamAttack()
{
	Entity *e;

	if (self->endX == 1)
	{
		self->flags |= INVULNERABLE;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->x = getMapStartX() + SCREEN_WIDTH / 2 - self->w / 2;

			playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

			self->flags &= ~NO_DRAW;

			self->dirY = 0;

			self->endX = 2;

			self->thinkTime = 30;
		}
	}

	else if (self->endX == 2)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add Grimlore's Beam");
			}

			self->maxThinkTime = 5;

			e->startX = self->x + self->offsetX;
			e->startY = self->y + self->offsetY;

			e->endX = getMapStartX() + SCREEN_WIDTH;
			e->endY = getMapFloor(self->x, self->y);

			e->x = getMapStartX();

			e->maxThinkTime = 0;

			e->action = &beamMove;

			e->draw = &beamDraw;

			e->targetX = playSoundToMap("sound/boss/grimlore/grimlore_beam", BOSS_CHANNEL, self->x, self->y, 0);

			e->head = self;

			e->thinkTime = 30;

			self->endX = 3;
		}
	}

	else if (self->endX == 3)
	{
		if (self->maxThinkTime <= 0)
		{
			self->thinkTime = 60;

			self->action = &swordDropTeleportAway;
		}
	}

	checkToMap(self);
}

static void beamMove()
{
	self->dirX += 0.25;

	self->x += self->dirX;

	if (self->x > self->endX)
	{
		self->thinkTime--;

		self->flags |= NO_DRAW;

		if (self->targetX != -1)
		{
			stopSound(self->targetX);

			self->targetX = -1;
		}

		if (self->thinkTime <= 0)
		{
			self->x = getMapStartX();

			self->thinkTime = 3;

			self->health = player.health;

			self->action = &beamExplosions;

			self->mental = 150;

			shakeScreen(MEDIUM, -1);
		}
	}
}

static void beamExplosions()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addExplosion(self->x + prand() % SCREEN_WIDTH, self->endY);

		e->mental = (self->mental % 10) == 0 ? 0 : 1;

		self->mental--;

		e->touch = &explosionTouch;

		e->y -= prand() % e->h;

		e->damage = 1;

		e->head = self;

		if (self->mental <= 0)
		{
			if (self->health > player.health && (prand() % 3 == 0))
			{
				setInfoBoxMessage(120, 255, 255, 255, _("Try using one of the artifacts to protect yourself..."));
			}

			self->head->maxThinkTime = 0;

			self->inUse = FALSE;

			shakeScreen(LIGHT, 0);
		}
	}
}

static int beamDraw()
{
	drawLine(self->startX, self->startY, self->x, self->endY, 220, 0, 0);

	return TRUE;
}

static void explosionTouch(Entity *other)
{
	if (other->element != FIRE)
	{
		if (other->type == PLAYER)
		{
			if (self->head->maxThinkTime == 0)
			{
				destroyInventoryItem();

				self->head->maxThinkTime = 1;
			}
		}

		entityTouch(other);
	}
}

static void magicMissileChargeWait()
{
	float radians;

	self->mental -= 2;

	if (self->mental <= 0)
	{
		self->head->thinkTime--;

		self->inUse = FALSE;
	}

	self->health += 8;

	radians = DEG_TO_RAD(self->health);

	self->x = (0 * cos(radians) - self->mental * sin(radians));
	self->y = (0 * sin(radians) + self->mental * cos(radians));

	self->x += self->startX;
	self->y += self->startY;
}

static void itemDestroyAttackInit()
{
	setEntityAnimation(self, "RAISE_ARMS_1");

	self->action = &itemDestroyAttack;

	self->thinkTime = 120;

	checkToMap(self);
}

static void itemDestroyAttack()
{
	int i, angle;
	Entity *e, *first, *prev;
	char weaponName[MAX_VALUE_LENGTH];

	self->thinkTime--;

	if (self->thinkTime == 60)
	{
		setEntityAnimation(self, "RAISE_ARMS_2");

		fadeFromColour(255, 0, 0, 60);
	}

	prev = first = NULL;

	if (self->thinkTime <= 0)
	{
		i = prand() % 3;

		switch (i)
		{
			case 0:
				STRNCPY(weaponName, "weapon/pickaxe", sizeof(weaponName));
			break;

			case 1:
				STRNCPY(weaponName, "weapon/wood_axe", sizeof(weaponName));
			break;

			default:
				STRNCPY(weaponName, "weapon/basic_sword", sizeof(weaponName));
			break;
		}

		angle = 0;

		for (i=0;i<8;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add an Item Destroyer");
			}

			loadProperties("boss/grimlore_item_destroyer", e);

			e->action = &rotateAroundPlayer;

			e->draw = &drawLoopingAnimationToMap;
			e->takeDamage = &itemDestroyerTakeDamage;
			e->touch = &entityTouch;
			e->die = &entityDieNoDrop;

			e->type = ENEMY;

			setEntityAnimation(e, "STAND");

			e->x = player.x + player.w / 2 - e->w / 2;
			e->y = player.y + player.h / 2 - e->h / 2;

			if (i == 0)
			{
				e->mental = 1;

				e->head = self;

				first = e;
			}

			else
			{
				e->head = first;

				prev->target = e;
			}

			e->active = FALSE;

			e->targetX = angle;

			e->endX = player.h / 2 + e->h;

			e->thinkTime = 600;

			angle += 45;

			STRNCPY(e->requires, weaponName, sizeof(e->requires));

			prev = e;
		}

		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Grimlore's Weapon Box");
		}

		loadProperties("boss/grimlore_weapon_box", e);

		setEntityAnimationByID(e, 0);

		e->x = self->x + self->w / 2 - e-> w / 2;
		e->y = self->y - e->h - 16;

		STRNCPY(e->requires, weaponName, sizeof(e->requires));

		e->action = &doNothing;

		e->draw = &drawLoopingAnimationToMap;

		if (self->target != NULL && self->target->inUse == TRUE)
		{
			self->target->inUse = FALSE;
		}

		self->target = e;

		self->maxThinkTime = 1;

		self->thinkTime = 180;

		self->action = &itemDestroyWait;
	}

	checkToMap(self);
}

static void rotateAroundPlayer()
{
	float radians;

	if (self->mental == 1)
	{
		if (self->head->maxThinkTime == 2)
		{
			self->active = TRUE;
		}
	}

	else
	{
		self->active = self->head->active;

		if (self->head->health <= 0)
		{
			self->die();

			return;
		}
	}

	self->targetX += 4;

	radians = DEG_TO_RAD(self->targetX);

	self->x = (0 * cos(radians) - self->endX * sin(radians));
	self->y = (0 * sin(radians) + self->endX * cos(radians));

	self->startX = player.x + player.w / 2 - self->w / 2;
	self->startY = player.y + player.h / 2 - self->h / 2;

	self->x += self->startX;
	self->y += self->startY;

	if (self->active == TRUE)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->endY = self->endX + 64;

			self->thinkTime = 60;

			self->action = &itemDestroyerExpand;
		}
	}
}

static void itemDestroyerExpand()
{
	float radians;

	self->endX++;

	if (self->endX > self->endY)
	{
		self->endX = self->endY;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &itemDestroyerRetract;
		}
	}

	self->targetX += 4;

	radians = DEG_TO_RAD(self->targetX);

	self->x = (0 * cos(radians) - self->endX * sin(radians));
	self->y = (0 * sin(radians) + self->endX * cos(radians));

	self->startX = player.x + player.w / 2 - self->w / 2;
	self->startY = player.y + player.h / 2 - self->h / 2;

	self->x += self->startX;
	self->y += self->startY;
}

static void itemDestroyerRetract()
{
	float radians;
	Entity *e;

	self->endX -= 8;

	if (self->endX <= 0)
	{
		self->x = player.x + player.w / 2 - self->w / 2;
		self->y = player.y + player.h / 2 - self->h / 2;

		if (self->mental == 1)
		{
			e = addExplosion(self->x, self->y);

			e->damage = 1;

			setPlayerConfused(600);

			self->head->maxThinkTime = 0;
		}

		self->inUse = FALSE;

		return;
	}

	self->targetX += 4;

	radians = DEG_TO_RAD(self->targetX);

	self->x = (0 * cos(radians) - self->endX * sin(radians));
	self->y = (0 * sin(radians) + self->endX * cos(radians));

	self->startX = player.x + player.w / 2 - self->w / 2;
	self->startY = player.y + player.h / 2 - self->h / 2;

	self->x += self->startX;
	self->y += self->startY;
}

static void itemDestroyWait()
{
	if (self->maxThinkTime == 1)
	{
		self->thinkTime--;

		if (self->thinkTime > 30)
		{
			if (self->thinkTime % 6 == 0)
			{
				self->target->health++;

				if (self->target->health >= 3)
				{
					self->target->health = 0;
				}

				self->target->face = self->target->face == LEFT ? RIGHT : LEFT;

				setEntityAnimationByID(self->target, self->target->health);

				playSoundToMap("sound/item/charge_beep", BOSS_CHANNEL, self->x, self->y, 0);
			}
		}

		else
		{
			setEntityAnimation(self->target, self->target->requires);

			self->maxThinkTime = 2;
		}
	}

	else if (self->maxThinkTime == 2)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->target->inUse = FALSE;

			self->target = NULL;

			self->thinkTime = 120;

			self->maxThinkTime = 3;
		}
	}

	if (self->maxThinkTime <= 0)
	{
		self->thinkTime--;

		if (self->target != NULL)
		{
			self->target->inUse = FALSE;
		}

		if (self->thinkTime <= 0)
		{
			self->action = self->health <= 0 ? &swordDropTeleportAway : &attackFinished;
		}
	}

	checkToMap(self);
}

static void itemDestroyerTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (self->active == TRUE && strcmpignorecase(self->requires, other->name) == 0)
	{
		temp = self->mental == 1 ? self : self->head;

		temp->health--;

		if (temp->health > 0)
		{
			while (temp != NULL)
			{
				setCustomAction(temp, &flashWhite, 6, 0, 0);
				setCustomAction(temp, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

				temp = temp->target;
			}

			enemyPain();
		}

		else
		{
			temp = self;

			self = self->mental == 1 ? self : self->head;

			self->head->maxThinkTime = 0;

			self->die();

			self = temp;
		}
	}

	else
	{
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		playSoundToMap("sound/common/dink", -1, self->x, self->y, 0);

		temp = self->mental == 1 ? self : self->head;

		while (temp != NULL)
		{
			temp->thinkTime = 0;

			temp->active = TRUE;

			setCustomAction(temp, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

			temp = temp->target;
		}
	}
}

static void destroyInventoryItem()
{
	int size, i, index;
	Entity *e;
	char itemName[MAX_VALUE_LENGTH];
	char *items[] = {
		"item/centurion_statue",
		"item/scorpion_statue",
		"item/tortoise_statue",
		"item/spider_statue",
		"item/health_potion",
		"weapon/lightning_sword",
		"weapon/lightning_sword_empty",
		"item/instruction_card",
		"weapon/normal_arrow",
		"weapon/flaming_arrow",
		"item/full_soul_bottle",
		"item/soul_bottle",
		"item/tortoise_shell",
		"item/summoner_staff",
		"item/flaming_arrow_potion",
		"item/resurrection_amulet",
		"item/keepsake",
		"weapon/legendary_sword"
	};

	size = sizeof(items) / sizeof(char *);

	for (i=0;i<size;i++)
	{
		index = prand() % size;

		e = getInventoryItemByName(items[index]);

		if (e != NULL)
		{
			STRNCPY(itemName, e->objectiveName, sizeof(itemName));

			removeInventoryItemByName(e->name);

			setInfoBoxMessage(60, 255, 255, 255, _("Your %s has been destroyed"), itemName);

			return;
		}
	}
}

static void crushAttackInit()
{
	Target *t;

	self->flags |= NO_DRAW;

	self->layer = BACKGROUND_LAYER;

	playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	t = getTargetByName("GRIMLORE_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Grimlore cannot find target");
	}

	self->x = t->x;
	self->y = t->y;

	self->thinkTime = 60;

	self->dirY = 0;

	self->flags |= FLY;

	self->action = &crushAttackAppear;

	checkToMap(self);
}

static void crushToMiddleAttackInit()
{
	Target *t;

	self->flags |= NO_DRAW;

	self->layer = BACKGROUND_LAYER;

	playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	t = getTargetByName("GRIMLORE_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Grimlore cannot find target");
	}

	self->x = t->x;
	self->y = t->y;

	self->thinkTime = 60;

	self->dirY = 0;

	self->flags |= FLY;

	self->action = &crushToMiddleAttackAppear;

	checkToMap(self);
}

static void crushAttackAppear()
{
	int startX;
	Entity *e;
	Target *t;

	self->thinkTime--;

	self->flags |= INVULNERABLE;

	if (self->thinkTime <= 0)
	{
		t = getTargetByName("GRIMLORE_FIST_TARGET");

		startX = getMapStartX();

		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Grimlore's Fist");
		}

		loadProperties("boss/grimlore_fist", e);

		e->face = LEFT;

		setEntityAnimation(e, "FIST_APPEAR");

		e->animationCallback = &fistAppearActivate;

		e->x = startX + SCREEN_WIDTH / 4 - e->w / 2;
		e->y = t->y;

		e->startX = startX;
		e->endX = startX + SCREEN_WIDTH - e->w;

		e->startY = e->y;

		e->flags |= LIMIT_TO_SCREEN;

		e->action = &fistMoveAbovePlayer;

		e->active = FALSE;

		e->dirX = e->speed * 1.5;

		e->draw = &drawLoopingAnimationToMap;
		e->takeDamage = &fistTakeDamage;

		e->touch = &entityTouch;

		e->head = self;

		e->mental = 600;

		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Grimlore's Fist");
		}

		loadProperties("boss/grimlore_fist", e);

		e->face = RIGHT;

		setEntityAnimation(e, "FIST_APPEAR");

		e->animationCallback = &fistAppearActivate;

		e->x = startX + SCREEN_WIDTH * 3 / 4 - e->w / 2;
		e->y = t->y;

		e->startX = startX;
		e->endX = startX + SCREEN_WIDTH - e->w;

		e->startY = e->y;

		e->flags |= LIMIT_TO_SCREEN;

		e->action = &fistMoveAbovePlayer;

		e->active = FALSE;

		e->dirX = e->speed * 1.5;

		e->draw = &drawLoopingAnimationToMap;
		e->takeDamage = &fistTakeDamage;

		e->touch = &entityTouch;

		e->head = self;

		e->mental = 600;

		self->maxThinkTime = 4;

		self->action = &crushAttackWait;
	}

	checkToMap(self);
}

static void crushToMiddleAttackAppear()
{
	int startX;
	Entity *e;
	Target *t;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		t = getTargetByName("GRIMLORE_FIST_TARGET");

		startX = getMapStartX();

		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Grimlore's Fist");
		}

		loadProperties("boss/grimlore_fist", e);

		e->face = LEFT;

		setEntityAnimation(e, "FIST_APPEAR");

		e->animationCallback = &fistAppearActivate;

		e->x = startX;
		e->y = t->y;

		e->endX = startX + SCREEN_WIDTH - e->w - 1;

		e->startY = e->y;

		e->flags |= LIMIT_TO_SCREEN;

		e->action = &fistMoveToMiddle;

		e->active = FALSE;

		e->mental = 0;

		e->draw = &drawLoopingAnimationToMap;
		e->takeDamage = &fistTakeDamage;

		e->touch = &entityTouch;

		e->head = self;

		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Grimlore's Fist");
		}

		loadProperties("boss/grimlore_fist", e);

		e->face = RIGHT;

		setEntityAnimation(e, "FIST_APPEAR");

		e->animationCallback = &fistAppearActivate;

		e->x = startX + SCREEN_WIDTH - e->w;
		e->y = t->y;

		e->endX = startX;

		e->startY = e->y;

		e->flags |= LIMIT_TO_SCREEN;

		e->action = &fistMoveToMiddle;

		e->active = FALSE;

		e->mental = 0;

		e->draw = &drawLoopingAnimationToMap;
		e->takeDamage = &fistTakeDamage;

		e->touch = &entityTouch;

		e->head = self;

		self->maxThinkTime = 4;

		self->action = &crushAttackWait;
	}

	checkToMap(self);
}

static void crushAttackWait()
{
	if (self->maxThinkTime <= 0)
	{
		self->thinkTime = 30;

		self->action = &swordDropTeleportAway;
	}

	checkToMap(self);
}

static void fistMoveAbovePlayer()
{
	float target;

	if (self->active == FALSE)
	{
		return;
	}

	target = player.x - self->w / 2 + player.w / 2;

	if (self->head->health <= 0 || self->mental <= 0)
	{
		self->dirX = 0;
		self->dirY = 0;

		self->action = &fistVanishWait;

		setEntityAnimation(self, "FIST_DISAPPEAR");

		self->animationCallback = &fistVanish;

		self->head->maxThinkTime--;

		return;
	}

	/* Move above the player */

	if (fabs(target - self->x) <= fabs(self->dirX))
	{
		self->targetY = self->y - self->h;

		self->thinkTime = self->face == LEFT ? 60 : 30;

		self->dirX = 0;

		self->action = &fistCrush;
	}

	else
	{
		self->dirX = self->speed * 1.5;

		self->x += target > self->x ? self->dirX : -self->dirX;

		if (self->x < self->startX)
		{
			self->x = self->startX;

			/* Crush if at the edge of the screen */

			if (self->x == getMapStartX())
			{
				self->thinkTime = 30;

				self->dirX = 0;

				self->action = &fistCrush;
			}
		}

		else if (self->x > self->endX)
		{
			self->x = self->endX;

			/* Crush if at the edge of the screen */

			if (self->x == getMapStartX() + SCREEN_WIDTH - self->w)
			{
				self->thinkTime = 30;

				self->dirX = 0;

				self->action = &fistCrush;
			}
		}
	}

	self->mental--;
}

static void fistCrush()
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

			shakeScreen(MEDIUM, 15);

			self->thinkTime = 15;

			self->action = &crushWait;

			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}
		}
	}

	self->mental--;
}

static void crushWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= FLY;

		if (self->y < self->startY)
		{
			self->y = self->startY;

			self->dirY = 0;
			self->action = &fistMoveAbovePlayer;

		}

		else
		{
			self->dirY = -4;
		}
	}

	checkToMap(self);

	self->mental--;
}

static void fistMoveToMiddle()
{
	int i;
	long onGround;

	if (self->active == FALSE)
	{
		return;
	}

	if (self->head->health <= 0)
	{
		self->dirX = 0;
		self->dirY = 0;

		self->action = &fistVanishWait;

		setEntityAnimation(self, "FIST_DISAPPEAR");

		self->animationCallback = &fistVanish;

		self->head->maxThinkTime--;

		return;
	}

	if (self->mental == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->flags &= ~FLY;

			onGround = (self->flags & ON_GROUND);

			checkToMap(self);

			if (onGround == 0 && (self->flags & ON_GROUND))
			{
				playSoundToMap("sound/common/crash", BOSS_CHANNEL, self->x, self->y, 0);

				shakeScreen(MEDIUM, 15);

				self->thinkTime = 15;

				if ((player.flags & ON_GROUND) && !(player.flags & INVULNERABLE))
				{
					setPlayerStunned(60);
				}

				for (i=0;i<20;i++)
				{
					addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
				}

				self->mental = 1;
			}
		}
	}

	else if (self->mental == 1)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->flags |= FLY;

			if (self->y < self->startY)
			{
				self->y = self->startY;

				self->dirY = 0;

				if ((self->face == LEFT && self->x >= self->endX) || (self->face == RIGHT && self->x <= self->endX))
				{
					self->thinkTime = 60;

					self->head->maxThinkTime--;

					self->action = &fistVanishWait;

					setEntityAnimation(self, "FIST_DISAPPEAR");

					self->animationCallback = &fistVanish;
				}

				else
				{
					self->mental = 2;

					if (self->face == LEFT)
					{
						self->targetX = self->x + self->w;

						if (self->targetX > self->endX)
						{
							self->targetX = self->endX;
						}

						self->dirX = self->speed * 1.5;
					}

					else
					{
						self->targetX = self->x - self->w;

						if (self->targetX < self->endX)
						{
							self->targetX = self->endX;
						}

						self->dirX = -self->speed * 1.5;
					}
				}
			}

			else
			{
				self->dirY = -4;
			}
		}
	}

	else if (self->mental == 2)
	{
		if ((self->face == LEFT && self->x >= self->targetX) || (self->face == RIGHT && self->x <= self->targetX))
		{
			self->mental = 0;

			self->x = self->targetX;

			self->dirX = 0;
		}
	}

	checkToMap(self);
}

static void fistVanishWait()
{
	if (self->active == FALSE && self->head->maxThinkTime <= 2)
	{
		self->head->maxThinkTime--;

		self->inUse = FALSE;
	}
}

static void fistTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	setCustomAction(self, &flashWhite, 6, 0, 0);

	/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

	if (other->type != PROJECTILE)
	{
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
	}

	temp = self->head;

	self = self->head;

	self->takeDamage(other, damage);

	self = temp;
}

static void fistAppearActivate()
{
	setEntityAnimation(self, "STAND");

	self->active = TRUE;
}

static void fistVanish()
{
	self->flags |= NO_DRAW;

	self->active = FALSE;
}

static void stunInit()
{
	setEntityAnimation(self, "STUNNED");

	self->thinkTime = 600;

	checkToMap(self);

	self->action = &stunWait;
}

static void stunWait()
{
	self->thinkTime--;

	if (self->endY == -100)
	{
		self->damage = 0;

		self->takeDamage = NULL;

		self->action = &bindInit;
	}

	else if (self->thinkTime <= 0)
	{
		self->health = self->maxHealth / 4;

		self->action = &swordDropTeleportAway;
	}

	checkToMap(self);
}

static void bindInit()
{
	Entity *e = addGrimloreSummonSpell(0, 0, "boss/grimlore_summon_spell");

	e->x = self->x + self->w / 2 - e->w / 2;
	e->y = self->y;

	e->mental = 1;

	e->health = 1;

	e->active = TRUE;

	self->action = &bindWait;

	checkToMap(self);
}

static void bindWait()
{
	checkToMap(self);
}

static void resetWeapons()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->endY == -99)
		{
			self->thinkTime = 30;

			setEntityAnimation(self, "RAISE_ARMS_2");

			self->mental = 0;

			self->endY = 0;
		}

		else if (self->mental == 0)
		{
			fadeFromColour(255, 0, 0, 60);

			setEntityAnimation(self, "STAND");

			removeInventoryItemByObjectiveName("Reflection Artifact");

			removeInventoryItemByObjectiveName("Protection Artifact");

			e = getEntityByObjectiveName("Reflection Artifact");

			if (e != NULL)
			{
				e->inUse = FALSE;
			}

			e = getEntityByObjectiveName("Protection Artifact");

			if (e != NULL)
			{
				e->inUse = FALSE;
			}

			addArmour();

			addSword();

			addShield();

			self->health = self->maxHealth;
		}

		else
		{
			setInfoBoxMessage(120, 255, 255, 255, _("Binding only works against weakened demons"));

			self->action = &attackFinished;
		}
	}

	checkToMap(self);
}

static void creditsMove()
{
	addArmour();

	addSword();

	addShield();

	self->creditsAction = &bossMoveToMiddle;
}
