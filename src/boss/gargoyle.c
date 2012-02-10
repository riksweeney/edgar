/*
Copyright (C) 2009-2012 Parallel Realities

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
#include "../custom_actions.h"
#include "../enemy/enemies.h"
#include "../enemy/rock.h"
#include "../entity.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../map.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/target.h"

extern Entity *self, player;

static void initialise(void);
static void addStoneCoat(void);
static void init(void);
static void doIntro(void);
static void introFinish(void);
static void attackFinished(void);
static void coatWait(void);
static void lanceThrowInit(void);
static void lanceThrowMoveToTarget(void);
static void lanceThrow(void);
static void lanceThrowWait(void);
static void lanceDrop(void);
static void lanceAttack1(void);

Entity *addGargoyle(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Gargoyle");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->die = &die;
	e->takeDamage = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case -1:
			setEntiyAnimation(self, "REACH_STONE");

			self->takeDamage = &dieTakeDamage;

			self->touch = &dieTouch;

			self->activate = &activate;

			self->action = &dieFinish;
		break;

		case 0:
			addStoneCoat();

			self->action = &initialise;
		break;

		default:
			self->action = &introFinish;
		break;
	}
}

static void initialise()
{
	Target *t;

	if (self->active == TRUE)
	{
		if (cameraAtMinimum())
		{
			if (self->mental == 0)
			{
				t = getTargetByName("GARGOYLE_INTRO_TARGET");

				if (t == NULL)
				{
					showErrorAndExit("Gargoyle cannot find target");
				}

				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

				self->dirX *= self->speed;
				self->dirY *= self->speed;

				self->thinkTime = 30;

				self->action = &doIntro;
			}
		}
	}

	checkToMap(self);
}

static void doIntro()
{
	int i;
	Entity *e;

	if (atTarget())
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->flags &= ~FLY;
		}
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
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

		self->action = &introFinish;
	}
}

static void introFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime = 0;

		self->mental = 2;

		setContinuePoint(FALSE, self->name, NULL);

		initBossHealthBar();

		facePlayer();

		playDefaultBossMusic();

		self->action = &attackFinished;
	}
}

static void attackFinished()
{
	self->thinkTime = 60;

	self->action = &entityWait;

	checkToMap(self);
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->target == NULL && self->maxThinkTime < 3)
		{
			self->action = &createLanceInit;
		}

		else
		{
			swtich (self->maxThinkTime)
			{
				case 0:
					switch (prand() % 4)
					{
						case 0:
							self->action = &lanceStab;
						break;

						case 1:
							self->action = &weaponRemoveBlastInit;
						break;

						default:
							self->action = &lanceThrowInit;
						break;
					}
				break;

				case 1:
					switch (prand() % 4)
					{
						/* Wind attack */
						/* Vertical throw 2 */
						/* Weapon remove blast */
					}
				break;

				case 2:
					switch (prand() % 4)
					{
						/* Invisible drop */
						/* Vertical throw 3 */
						/* Weapon remove blast */
					}
				break;

				default:
					switch (prand() % 4)
					{
						/* Drop attack */
						/* Weapon remove blast */
						/* Punch */
					}
				break;
			}
		}
	}

	checkToMap(self);
}

static void lanceThrowInit()
{
	Target *t = getTargetByName("GARGOYLE_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Gargoyle cannot find target");
	}

	self->targetX = self->x;
	self->targetY = t->y;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= self->speed;
	self->dirY *= self->speed;

	self->flags |= FLY;

	self->action = &lanceThrowMoveToTarget;

	checkToMap(self);
}

static void lanceThrowMoveToTarget()
{
	if (atTarget())
	{
		self->thinkTime = 30;

		self->action = &lanceThrow;
	}

	checkToMap(self);
}

static void lanceThrow()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "LANCE_THROW");

		self->target->mental = -1;

		self->action = &lanceThrowWait;

		self->mental = 1;
	}

	checkToMap(self);
}

static void lanceThrowWait()
{
	if (self->mental == 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void createLanceInit()
{
	self->flags &= ~FLY;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->thinkTime = 15;

		self->action = &createLance;
	}
}

static void createLance()
{
	Entity *e;

	setEntityAnimation(self, "CREATE_LANCE");

	self->mental = 1;

	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Gargoyle");
	}

	switch (self->maxThinkTime)
	{
		case 0:
			loadProperties("boss/gargoyle_lance_1", e);
		break;

		case 1:
			loadProperties("boss/gargoyle_lance_2", e);
		break;

		default:
			loadProperties("boss/gargoyle_lance_3", e);
		break;
	}

	e->action = &lanceAppearWait;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->head = self;

	setEntityAnimation(e, "LANCE_APPEAR");

	e->animationCallback = &lanceAppearFinish;

	self->target = e;

	e->maxThinkTime = 0;

	self->action = &createLanceWait;
}

static void createLanceWait()
{
	if (self->mental == 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void lanceAppearWait()
{
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

static void lanceAppearFinish()
{
	self->head->mental = 0;

	self->head->health = self->head->maxHealth;

	self->action = &lanceWait;
}

static void lanceWait()
{
	self->face = self->head->face;

	setEntityAnimation(self, getAnimationTypeAtIndex(self->head));

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;

	if (self->mental == -1)
	{
		self->action = &lanceDrop;
	}
}

static void lanceDrop()
{
	self->flags &= ~FLY;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		switch (self->maxThinkTime)
		{
			case 0:
				self->action = &lanceAttack1;
			break;

			case 1:
				self->action = &lanceAttack2;
			break;

			default:
				self->action = &lanceAttack3;
			break;
		}

		self->thinkTime = 30;
	}
}

static void weaponRemoveBlastInit()
{
	self->flags &= ~FLY;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->thinkTime = 120;

		self->action = &weaponRemoveBlast;
	}
}

static void weaponRemoveBlast()
{
	Entity *e;

	facePlayer();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addProjectile("boss/gargoyle_weapon_remove_blast", self, self->x, self->y, self->face == LEFT ? -8 : 8, 0);

		e->face = self->face;

		e->damage = player.health / 2;

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - e->w - e->offsetX;
		}

		else
		{
			e->x = self->x + e->offsetX;
		}

		e->y = self->y + e->offsetY;

		e->touch = &blastRemoveWeapon;

		e->flags |= FLY;

		e->thinkTime = 1200;

		self->thinkTime = 120;

		self->action = &weaponRemoveBlastFinish;
	}

	checkToMap(self);
}

static void blastRemoveWeapon(Entity *other)
{
	Entity *e;

	if (other->type == PLAYER)
	{
		e = removePlayerWeapon();

		if (e != NULL)
		{
			e->x = self->x;
			e->y = self->y;

			e->dirX = (6 + prand() % 3) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = -12;

			setCustomAction(e, &invulnerable, 120, 0, 0);

			addExitTrigger(e);
		}

		e = removePlayerShield();

		if (e != NULL)
		{
			e->x = self->x;
			e->y = self->y;

			e->dirX = (6 + prand() % 3) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = -12;

			setCustomAction(e, &invulnerable, 120, 0, 0);

			addExitTrigger(e);
		}

		setCustomAction(self->target, &invulnerable, 60, 0, 0);

		setPlayerStunned(30);

		self->target->dirX = (6 + prand() % 3) * (self->dirX < 0 ? -1 : 1);
		self->target->dirY = -8;

		self->inUse = FALSE;
	}
}

static void weaponRemoveBlastFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{
	if (self->flags & INVULNERABLE)
	{
		return;
	}

	/* Take minimal damage from bombs */

	if (other->type == EXPLOSION)
	{
		damage = 1;
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

		/* Don't die if you've still got lances to wield */

		else if (self->maxThinkTime >= 3)
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
	}

	if (other->type == PROJECTILE)
	{
		temp = self;

		self = other;

		self->die();

		self = temp;
	}
}

static void lanceAttack1()
{
	int i;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime = 0;

		self->mental = 2;

		self->action = &createLightningOrb;
	}

	checkToMap(self);
}

static void createLightningOrb()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a lightning orb");
		}

		loadProperties("boss/gargoyle_lightning_orb", e);

		e->x = self->x;
		e->y = self->y;

		e->action = &orbMoveToTop;

		e->draw = &drawLoopingAnimationToMap;
		e->touch = NULL;
		e->takeDamage = NULL;

		e->type = ENEMY;

		e->head = self;

		setEntityAnimation(e, "STAND");

		self->mental--;

		self->maxThinkTime++;

		if (self->mental <= 0)
		{
			self->action = &lanceAttack1Wait;
		}

		else
		{
			self->thinkTime = 15;
		}
	}

	checkToMap(self);
}

static void lanceAttack1Wait()
{
	if (self->maxThinkTime <= 0)
	{
		self->thinkTime = 30;

		self->action = &lanceAttackTeleportAway;
	}

	checkToMap(self);
}

static void orbMoveToTop()
{
	self->y -= self->speed;

	if (self->y <= self->targetY)
	{
		self->action = &orbFollowPlayer;
	}
}

static void orbFollowPlayer()
{
	float target = player.x - self->w / 2 + player.w / 2;

	/* Move above the player */

	if (fabs(target - self->x) <= fabs(self->dirX))
	{
		self->thinkTime = 30;

		self->dirX = 0;

		self->action = &orbCastLightning;
	}

	else
	{
		self->dirX = self->speed * 1.5;

		self->x += target > self->x ? self->dirX : -self->dirX;

		if (self->x < self->startX)
		{
			self->x = self->startX;

			self->thinkTime = 30;

			self->dirX = 0;

			self->action = &orbCastLightning;
		}

		else if (self->x > self->endX)
		{
			self->x = self->endX;

			self->thinkTime = 30;

			self->dirX = 0;

			self->action = &orbCastLightning;
		}
	}
}

static void orbCastLightning()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		playSoundToMap("sound/enemy/thunder_cloud/lightning.ogg", -1, self->x, self->y, 0);

		for (i=self->endY-32;i>=self->startY;i-=32)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add lightning");
			}

			loadProperties("enemy/lightning", e);

			setEntityAnimation(e, "STAND");

			e->x = self->x + self->w / 2 - e->w / 2;
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

		self->action = &castLightningFinish;

		self->thinkTime = 30;
	}
}

static void castLightningFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental--;

		if (self->mental <= 0)
		{
			self->head->maxThinkTime--;

			self->inUse = FALSE;
		}

		else
		{
			self->action = &followPlayer;
		}

		self->thinkTime = 30;
	}
}

void lightningWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}

static void lanceAttack2()
{
	self->thinkTime--;

	checkToMap(self);
}

static void becomeMiniGargoyleInit()
{
	int i;

	self->flags |= NO_DRAW;

	self->touch = NULL;

	self->mental = 0;

	self->endX = 0;

	for (i=0;i<20;i++)
	{
		e = addEnemy("enemy/mini_gargoyle", 0, 0);

		e->head = self;

		e->thinkTime = 30 + prand() % 120;

		self->mental++;

		self->endX++;
	}

	checkToMap(self);
}

static void becomeMiniGargoyleWait()
{
	if (self->endX <= 0)
	{
		self->action = &becomeMiniGargoyleFinish;
	}

	checkToMap(self);
}

static void becomeMiniGargoyleFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~NO_DRAW;

		self->touch = &entityTouch;

		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void lanceAttackTeleportAway()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= NO_DRAW;

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

		self->face = self->head->face;

		setEntityAnimation(self, getAnimationTypeAtIndex(self->head));

		if (self->face == LEFT)
		{
			self->x = self->head->x + self->head->w - self->w - self->offsetX;
		}

		else
		{
			self->x = self->head->x + self->offsetX;
		}

		self->y = self->head->y + self->offsetY;

		self->thinkTime = 30;

		self->action = &lanceAttackTeleportFinish;
	}
}

static void lanceAttackTeleportFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~NO_DRAW;

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

		self->action = &lanceWait;

		self->head->mental = 0;
	}
}

static void addStoneCoat()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Gargoyle Stone Coat");
	}

	loadProperties("boss/gargoyle_stone", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &coatWait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;

	e->head = self;

	setEntityAnimation(e, "STAND");

	return e;
}

static void coatWait()
{
	self->face = self->head->face;

	setEntityAnimation(self, getAnimationTypeAtIndex(self->head));

	if (self->head->mental == 1)
	{
		self->alpha--;

		if (self->alpha <= 0)
		{
			self->head->mental = 0;

			self->inUse = FALSE;
		}
	}

	else if (self->head->mental == 2)
	{
		self->alpha++;

		if (self->alpha >= 255)
		{
			self->head->mental = 0;

			self->mental = 0;
		}
	}

	self->x = self->head->x;
	self->y = self->head->y;

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static void die()
{
	self->flags &= ~FLY;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->damage = 0;

		self->takeDamage = NULL;

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

		addStoneCoat();

		self->health = 0;

		self->mental = 2;

		self->thinkTime = 60;

		self->action = &dieWait;

		self->startX = self->x;
	}
}

static void dieWait()
{
	setEntiyAnimation(self, "REACH");

	self->x = self->startX + sin(DEG_TO_RAD(self->startY)) * 4;

	self->startY += 90;

	if (self->startY >= 360)
	{
		self->startY = 0;
	}

	if (self->mental == 0)
	{
		self->x = self->startX;

		self->mental = -1;

		setEntiyAnimation(self, "REACH_STONE");

		self->takeDamage = &dieTakeDamage;

		self->touch = &dieTouch;

		self->activate = &activate;

		self->health = 8;

		self->action = &dieFinish;

		clearContinuePoint();

		increaseKillCount();

		freeBossHealthBar();

		fadeBossMusic();

		fireTrigger(self->objectiveName);

		fireGlobalTrigger(self->objectiveName);
	}
}

static void dieTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
	}

	else
	{
		entityTouch(other);
	}
}

static void activate(int val)
{
	runScript(self->requires);
}

static void dieFinish()
{
	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (strcmpignorecase("weapon/pickaxe", other->name) == 0)
	{
		self->health -= damage;

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		if (self->health <= 0)
		{
			self->action = &stoneDie;
		}
	}

	else
	{
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		playSoundToMap("sound/common/dink.ogg", -1, self->x, self->y, 0);

		if (other->reactToBlock != NULL)
		{
			temp = self;

			self = other;

			self->reactToBlock(temp);

			self = temp;
		}

		if (other->type != PROJECTILE && prand() % 10 == 0)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
		}

		damage = 0;
	}
}

static void stoneDie()
{
	int i;

	Entity *e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

	e->dirY = ITEM_JUMP_HEIGHT;

	for (i=0;i<8;i++)
	{
		e = addTemporaryItem("boss/gargoyle_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += self->w / 2 - e->w / 2;
		e->y += self->h / 2 - e->h / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->thinkTime = 60 + (prand() % 60);
	}

	self->inUse = FALSE;
}
