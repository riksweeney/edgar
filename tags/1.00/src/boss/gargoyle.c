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
#include "../credits.h"
#include "../custom_actions.h"
#include "../enemy/enemies.h"
#include "../enemy/rock.h"
#include "../entity.h"
#include "../event/global_trigger.h"
#include "../event/script.h"
#include "../event/trigger.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
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

extern Input input;
extern Entity *self, player;

static void initialise(void);
static void addStoneCoat(void);
static void init(void);
static void doIntro(void);
static void introFinish(void);
static void attackFinished(void);
static void attackFinishedMoveUp(void);
static void attackFinishedMoveHorizontal(void);
static void attackFinished(void);
static void entityWait(void);
static void takeDamage(Entity *, int);
static void stoneTakeDamage(Entity *, int);
static void die(void);
static void dieWait(void);
static void dieFinish(void);
static void activate(int);
static void stoneTouch(Entity *);
static void stoneDie(void);
static void coatWait(void);
static void createLanceInit(void);
static void createLance(void);
static void createLanceWait(void);
static void lanceAppearWait(void);
static void lanceAppearFinish(void);
static void lanceWait(void);
static void lanceThrowInit(void);
static void lanceThrowMoveToTarget(void);
static void lanceThrow(void);
static void lanceThrowWait(void);
static void lanceDrop(void);
static void lanceStabInit(void);
static void lanceStabMoveToTarget(void);
static void lanceStab(void);
static void lanceStabReactToBlock(Entity *);
static void lanceStabFinish(void);
static void lanceAttack1(void);
static void lanceAttack1Wait(void);
static void lanceAttack2(void);
static void lanceAttack3(void);
static void lanceAttackTeleportFinish(void);
static void lanceDie(void);
static void weaponRemoveBlastInit(void);
static void weaponRemoveBlast(void);
static void blastRemoveWeapon(Entity *);
static void weaponRemoveBlastFinish(void);
static void createLightningOrb(void);
static void lightningGridAttackInit(void);
static void lightningGridAttack(void);
static void lightningGridAttackWait(void);
static void orbMoveToTop(void);
static void orbMoveToTarget(void);
static void orbFollowPlayer(void);
static void orbCastLightning1(void);
static void orbCastLightning2(void);
static void orbCastLightningFinish1(void);
static void orbCastLightningFinish2(void);
static void lightningWait(void);
static void becomeMiniGargoyleInit(void);
static void becomeMiniGargoyleWait(void);
static void becomeMiniGargoyleFinish(void);
static void addExitTrigger(Entity *);
static void petrifyAttackInit(void);
static void petrifyAttack(void);
static void petrifyAttackWait(void);
static void invisibleAttackInit(void);
static void becomeInvisible(void);
static void invisibleAttackMoveToTop(void);
static void invisibleAttackMoveToTop(void);
static void invisibleAttackFollowPlayer(void);
static void invisibleDrop(void);
static void invisibleDropWait(void);
static void bridgeDestroyInit(void);
static void bridgeDestroyMoveToTarget(void);
static void bridgeDestroyFollowPlayer(void);
static void bridgeDestroy(void);
static void bridgeDestroyWait(void);
static void bridgeDestroyFinish(void);
static void lanceAttack2(void);
static void lanceAttack2Wait(void);
static void lanceDestroyBridge(void);
static void lanceFallout(void);
static void lanceAttack3(void);
static void fakeLanceDropInit(void);
static void fakeLanceDropAppear(void);
static void fakeLanceDropWait(void);
static void fakeLanceDrop(void);
static void fakeLanceDropExplodeWait(void);
static void fakeLanceDie(void);
static void lanceExplode(void);
static void splitInHalfInit(void);
static void splitInHalf(void);
static void splitInHalfMove(void);
static void cloneCheck(void);
static void cloneDie(void);
static void dropAttackInit(void);
static void dropAttackMoveToTop(void);
static void dropAttackFollowPlayer(void);
static void dropAttack(void);
static void creditsMove(void);
static void addLance(void);
static void lanceCreditsMove(void);
static void fallout(void);
static void falloutWait(void);

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
	e->touch = &entityTouch;
	e->die = &die;
	e->takeDamage = NULL;
	e->fallout = &fallout;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "FACE_FRONT_CROUCH");

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case -1:
			self->flags &= ~FLY;

			setEntityAnimation(self, "REACH_STONE");

			self->takeDamage = &stoneTakeDamage;

			self->touch = &stoneTouch;

			self->activate = &activate;

			self->action = &dieFinish;
		break;

		case 0:
			addStoneCoat();

			self->action = &initialise;
		break;

		default:
			setEntityAnimation(self, "FACE_FRONT_CROUCH");

			self->action = &introFinish;
		break;
	}
}

static void initialise()
{
	if (self->active == TRUE)
	{
		self->action = &doIntro;
	}

	checkToMap(self);
}

static void doIntro()
{
	int i;
	Entity *e;

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
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

		self->thinkTime = 60;

		self->action = &introFinish;
	}

	checkToMap(self);
}

static void introFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental == 0)
		{
			playSoundToMap("sound/boss/gargoyle/gargoyle_stone_to_flesh.ogg", BOSS_CHANNEL, self->x, self->y, -1);

			self->mental = 1;
		}

		else if (self->mental == 2)
		{
			stopSound(BOSS_CHANNEL);

			playDefaultBossMusic();

			self->maxThinkTime = 0;

			setContinuePoint(FALSE, self->name, NULL);

			initBossHealthBar();

			self->takeDamage = &takeDamage;

			self->action = &entityWait;

			self->thinkTime = 90;

			self->flags |= LIMIT_TO_SCREEN;
		}
	}
}

static void attackFinished()
{
	Target *t = getTargetByName("GARGOYLE_BOTTOM_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Gargoyle cannot find target");
	}

	facePlayer();

	setEntityAnimation(self, "FLY");

	self->flags |= (FLY|UNBLOCKABLE);

	self->targetX = self->x;
	self->targetY = t->y;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= 3;
	self->dirY *= 3;

	self->thinkTime = 30;

	self->action = &attackFinishedMoveUp;

	checkToMap(self);
}

static void attackFinishedMoveUp()
{
	if (atTarget())
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->targetX = getMapStartX() + prand() % (SCREEN_WIDTH - self->w);
			self->targetY = self->y;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= 3;
			self->dirY *= 3;

			self->thinkTime = 30;

			self->action = &attackFinishedMoveHorizontal;
		}
	}

	checkToMap(self);
}

static void attackFinishedMoveHorizontal()
{
	if (atTarget())
	{
		self->thinkTime = 0;

		self->flags &= ~UNBLOCKABLE;

		self->action = &entityWait;
	}

	facePlayer();

	checkToMap(self);
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		self->startX = getMapStartX();
		self->endX   = getMapStartX() + SCREEN_WIDTH - self->w;

		if ((self->target == NULL || self->target->inUse == FALSE) && self->maxThinkTime < 3)
		{
			self->action = &createLanceInit;
		}

		else
		{
			switch (self->maxThinkTime)
			{
				case 0:
					if (self->health == self->maxHealth / 4)
					{
						self->action = &lanceThrowInit;
					}

					else
					{
						if (player.element == SLIME)
						{
							self->action = &petrifyAttackInit;
						}

						else
						{
							switch (prand() % 3)
							{
								case 0:
									self->action = &lanceStabInit;
								break;

								case 1:
									self->action = &weaponRemoveBlastInit;
								break;

								default:
									self->action = &lightningGridAttackInit;
								break;
							}
						}
					}
				break;

				case 1:
					if (self->health == self->maxHealth / 4)
					{
						self->action = &lanceThrowInit;
					}

					else
					{
						switch (prand() % 3)
						{
							case 0:
								self->action = &lanceStabInit;
							break;

							case 1:
								self->action = &petrifyAttackInit;
							break;

							default:
								self->action = &bridgeDestroyInit;
							break;
						}
					}
				break;

				case 2:
					if (self->health == self->maxHealth / 4)
					{
						self->action = &lanceThrowInit;
					}

					else
					{
						if (player.element == SLIME)
						{
							self->action = &petrifyAttackInit;
						}

						else
						{
							switch (prand() % 4)
							{
								case 0:
									self->action = &lanceStabInit;
								break;

								case 1:
									self->action = &weaponRemoveBlastInit;
								break;

								case 2:
									self->action = &petrifyAttackInit;
								break;

								default:
									self->action = &invisibleAttackInit;
								break;
							}
						}
					}
				break;

				case 3:
					self->action = &splitInHalfInit;
				break;

				default:
					switch (prand() % 2)
					{
						case 0:
							self->action = &dropAttackInit;
						break;

						case 1:
							self->action = &weaponRemoveBlastInit;
						break;
					}
				break;
			}
		}
	}

	checkToMap(self);
}

static void splitInHalfInit()
{
	self->maxThinkTime = 4;

	setEntityAnimation(self, "DROP_ATTACK");

	self->thinkTime = 60;

	self->action = &splitInHalf;

	checkToMap(self);
}

static void splitInHalf()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Gargoyle's Duplicate");
		}

		loadProperties(self->name, e);

		setEntityAnimation(e, "DROP_ATTACK");

		e->x = self->x;
		e->y = self->y;

		e->action = &splitInHalfMove;

		e->draw = &drawLoopingAnimationToMap;

		e->touch = &entityTouch;

		e->takeDamage = &entityTakeDamageNoFlinch;

		e->die = &cloneDie;

		e->pain = &enemyPain;

		e->type = ENEMY;

		e->head = self;

		e->flags |= LIMIT_TO_SCREEN|DO_NOT_PERSIST|FLY;

		e->targetX = self->x - self->w;

		e->targetY = self->y;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->maxThinkTime = self->maxThinkTime;

		e->thinkTime = 0;

		e->startX = getMapStartX();
		e->endX   = getMapStartX() + SCREEN_WIDTH - e->w;

		e->health = self->maxHealth;

		self->action = &splitInHalfMove;

		self->targetX = self->x + self->w;

		self->targetY = self->y;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->target = e;

		self->thinkTime = 0;
	}
}

static void splitInHalfMove()
{
	checkToMap(self);

	if (atTarget() || self->dirX == 0)
	{
		if (self->head != NULL && self->head->dirX == 0)
		{
			self->flags &= ~NO_DRAW;

			self->action = &attackFinished;
		}

		else if (self->target != NULL && self->target->dirX == 0)
		{
			self->flags &= ~NO_DRAW;

			self->action = &attackFinished;
		}
	}

	else
	{
		self->thinkTime++;

		if (self->thinkTime % 2 == 0)
		{
			self->flags ^= NO_DRAW;
		}
	}
}

static void dropAttackInit()
{
	Target *t;

	t = getTargetByName("GARGOYLE_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Gargoyle cannot find target");
	}

	self->targetX = self->x;
	self->targetY = t->y;

	self->flags |= FLY;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= 5;
	self->dirY *= 5;

	self->action = &dropAttackMoveToTop;

	checkToMap(self);

	cloneCheck();
}

static void dropAttackMoveToTop()
{
	if (atTarget())
	{
		setEntityAnimation(self, "DROP_ATTACK_READY");

		self->action = &dropAttackFollowPlayer;
	}

	checkToMap(self);

	cloneCheck();
}

static void dropAttackFollowPlayer()
{
	float target;

	target = player.x - self->w / 2 + player.w / 2;

	/* Move above the player */

	if (fabs(target - self->x) <= fabs(self->dirX))
	{
		self->targetY = self->y - self->h;

		self->dirX = 0;

		self->thinkTime = 30;

		self->action = &dropAttack;
	}

	else
	{
		self->dirX = self->speed * 1.5;

		self->x += target > self->x ? self->dirX : -self->dirX;

		if (self->x < self->startX)
		{
			self->x = self->startX;

			/* Drop if at the edge of the screen */

			if (self->x == getMapStartX())
			{
				self->dirX = 0;

				self->thinkTime = 30;

				self->action = &dropAttack;
			}
		}

		else if (self->x > self->endX)
		{
			self->x = self->endX;

			/* Drop if at the edge of the screen */

			if (self->x == getMapStartX() + SCREEN_WIDTH - self->w)
			{
				self->dirX = 0;

				self->thinkTime = 30;

				self->action = &dropAttack;
			}
		}
	}

	cloneCheck();
}

static void dropAttack()
{
	int i;
	long onGround = self->flags & ON_GROUND;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "DROP_ATTACK");

		self->flags &= ~FLY;

		if (landedOnGround(onGround) == TRUE)
		{
			playSoundToMap("sound/enemy/red_grub/thud.ogg", -1, self->x, self->y, 0);

			for (i=0;i<30;i++)
			{
				e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

				if (e != NULL)
				{
					e->y -= prand() % e->h;
				}
			}

			self->thinkTime = 30;
		}

		if (self->standingOn != NULL || (self->flags & ON_GROUND))
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->action = &attackFinished;
			}
		}
	}

	checkToMap(self);

	cloneCheck();
}

static void lightningGridAttackInit()
{
	self->flags &= ~FLY;

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime = 30;

		self->action = &lightningGridAttack;
	}

	checkToMap(self);
}

static void lightningGridAttack()
{
	int i, x, orbCount;
	Target *t;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "CREATE_LANCE");

		self->mental = 0;

		self->endY = 0;

		orbCount = 6;

		x = getMapStartX();

		t = getTargetByName("GARGOYLE_TOP_TARGET");

		if (t == NULL)
		{
			showErrorAndExit("Gargoyle cannot find target");
		}

		for (i=0;i<orbCount+1;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add a lightning orb");
			}

			loadProperties("boss/gargoyle_lightning_orb", e);

			e->x = self->x + self->w / 2 - e->w / 2;
			e->y = self->y + self->h / 2 - e->h / 2;

			e->targetX = x - e->w / 2;
			e->targetY = t->y;

			e->endY = self->y + self->h;

			calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

			e->dirX *= 12;
			e->dirY *= 12;

			e->action = &orbMoveToTarget;

			e->draw = &drawLoopingAnimationToMap;
			e->touch = NULL;
			e->takeDamage = NULL;

			e->type = ENEMY;

			e->thinkTime = 30;

			e->mental = 1;

			e->health = i == 0 ? -1 : 0;

			e->head = self;

			x += SCREEN_WIDTH / orbCount;

			self->mental++;

			self->endY++;
		}

		setEntityAnimation(e, "STAND");

		self->thinkTime = 60;

		self->action = &lightningGridAttackWait;
	}

	checkToMap(self);
}

static void orbMoveToTarget()
{
	if (atTarget())
	{
		if (self->mental == 1)
		{
			self->head->endY--;

			self->mental = 0;
		}

		else if (self->head->endY <= 0)
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->startY = self->targetY;

				self->action = &orbCastLightning1;
			}
		}
	}

	checkToMap(self);
}

static void orbCastLightning1()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->health == -1)
		{
			playSoundToMap("sound/enemy/thunder_cloud/lightning.ogg", -1, self->x, self->y, 0);
		}

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

			e->thinkTime = 90;
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

		self->action = &orbCastLightningFinish1;

		self->thinkTime = 90;
	}
}

static void orbCastLightningFinish1()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->head->mental--;

		self->inUse = FALSE;
	}
}

static void lightningGridAttackWait()
{
	if (self->mental <= 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &attackFinished;
		}
	}

	checkToMap(self);
}

static void lanceStabInit()
{
	self->thinkTime = 30;

	self->action = &lanceStabMoveToTarget;

	checkToMap(self);
}

static void lanceStabMoveToTarget()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		facePlayer();

		calculatePath(self->x, self->y, player.x + player.w / 2 - self->w / 2, player.y + player.h / 2 - self->h / 2, &self->dirX, &self->dirY);

		self->dirX *= 16;
		self->dirY *= 16;

		self->action = &lanceStab;

		self->reactToBlock = &lanceStabReactToBlock;

		playSoundToMap("sound/boss/gargoyle/gargoyle_lance_stab.ogg", -1, self->x, self->y, 0);
	}

	checkToMap(self);
}

static void lanceStab()
{
	if (self->dirX == 0 || self->standingOn != NULL)
	{
		self->flags &= ~FLY;

		self->dirX = self->face == RIGHT ? -5 : 5;
		self->dirY = -6;

		self->action = &lanceStabFinish;

		self->thinkTime = 30;
	}

	checkToMap(self);
}

static void lanceStabReactToBlock(Entity *other)
{
	self->dirX = 0;
}

static void lanceStabFinish()
{
	int i;
	long onGround;
	Entity *e;

	self->reactToBlock = NULL;

	onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (landedOnGround(onGround) == TRUE)
	{
		setEntityAnimation(self, "STAND");

		for (i=0;i<5;i++)
		{
			e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

			if (e != NULL)
			{
				e->y -= prand() % e->h;
			}
		}
	}

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		self->dirX = 0;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &attackFinished;
		}
	}
}

static void invisibleAttackInit()
{
	self->flags &= ~FLY;

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		self->action = &becomeInvisible;

		self->endY = self->y + self->h;
	}

	checkToMap(self);
}

static void becomeInvisible()
{
	Target *t;

	setEntityAnimation(self, "CREATE_LANCE");

	self->alpha -= 3;

	if (self->alpha <= 0)
	{
		self->alpha = 255;

		self->flags |= NO_DRAW;

		self->mental = 1 + prand() % 3;

		t = getTargetByName("GARGOYLE_TOP_TARGET");

		if (t == NULL)
		{
			showErrorAndExit("Gargoyle cannot find target");
		}

		self->targetX = self->x;
		self->targetY = t->y;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 5;
		self->dirY *= 5;

		self->flags |= FLY;

		self->action = &invisibleAttackMoveToTop;

		checkToMap(self);
	}
}

static void invisibleAttackMoveToTop()
{
	if (atTarget())
	{
		setEntityAnimation(self, "DROP_ATTACK");

		self->action = &invisibleAttackFollowPlayer;
	}

	checkToMap(self);
}

static void invisibleAttackFollowPlayer()
{
	float target;

	target = player.x - self->w / 2 + player.w / 2;

	/* Move above the player */

	if (fabs(target - self->x) <= fabs(self->dirX))
	{
		self->targetY = self->y - self->h;

		self->thinkTime = 60;

		self->dirX = 0;

		self->action = &invisibleDrop;
	}

	else
	{
		self->dirX = self->speed * 1.5;

		self->x += target > self->x ? self->dirX : -self->dirX;

		if (self->x < self->startX)
		{
			self->x = self->startX;

			/* Drop if at the edge of the screen */

			if (self->x == getMapStartX())
			{
				self->thinkTime = 60;

				self->dirX = 0;

				self->action = &invisibleDrop;
			}
		}

		else if (self->x > self->endX)
		{
			self->x = self->endX;

			/* Drop if at the edge of the screen */

			if (self->x == getMapStartX() + SCREEN_WIDTH - self->w)
			{
				self->thinkTime = 60;

				self->dirX = 0;

				self->action = &invisibleDrop;
			}
		}
	}
}

static void invisibleDrop()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->standingOn != NULL || (self->flags & ON_GROUND))
		{
			shakeScreen(LIGHT, 15);

			for (i=0;i<30;i++)
			{
				e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

				if (e != NULL)
				{
					e->y -= prand() % e->h;
				}
			}

			playSoundToMap("sound/enemy/red_grub/thud.ogg", -1, self->x, self->y, 0);

			self->flags &= ~NO_DRAW;

			self->mental--;

			self->thinkTime = self->mental <= 0 ? 30 : 60;

			self->action = &invisibleDropWait;
		}

		self->flags &= ~FLY;
	}

	else
	{
		if (self->thinkTime % 2 == 0)
		{
			e = addSmoke(self->x + self->w / 2, self->endY, "decoration/dust");

			if (e != NULL)
			{
				e->dirX = -(10 + prand() % 30);

				e->dirX /= 10;

				e->y -= prand() % e->h;
			}

			e = addSmoke(self->x + self->w / 2, self->endY, "decoration/dust");

			if (e != NULL)
			{
				e->dirX = (10 + prand() % 30);

				e->dirX /= 10;

				e->y -= prand() % e->h;
			}
		}
	}

	checkToMap(self);
}

static void invisibleDropWait()
{
	Target *t;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental <= 0)
		{
			self->action = &attackFinished;
		}

		else
		{
			t = getTargetByName("GARGOYLE_TOP_TARGET");

			if (t == NULL)
			{
				showErrorAndExit("Gargoyle cannot find target");
			}

			self->flags |= NO_DRAW;

			self->targetX = self->x;
			self->targetY = t->y;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= self->speed;
			self->dirY *= self->speed;

			self->flags |= FLY;

			self->action = &invisibleAttackMoveToTop;
		}
	}

	checkToMap(self);
}

static void bridgeDestroyInit()
{
	Target *t = getTargetByName("GARGOYLE_MID_TARGET");

	setEntityAnimation(self, "LANCE_THROW_READY");

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

	self->action = &bridgeDestroyMoveToTarget;

	checkToMap(self);
}

static void bridgeDestroyMoveToTarget()
{
	if (atTarget())
	{
		self->mental = 5;

		self->action = &bridgeDestroyFollowPlayer;
	}

	checkToMap(self);
}

static void bridgeDestroyFollowPlayer()
{
	float target;

	target = player.x - self->w / 2 + player.w / 2;

	/* Move above the player */

	if (fabs(target - self->x) <= fabs(self->dirX))
	{
		self->targetY = self->y - self->h;

		self->thinkTime = 15;

		self->dirX = 0;

		if (player.y < getMapStartY() + SCREEN_HEIGHT)
		{
			self->action = &bridgeDestroy;
		}
	}

	else
	{
		self->dirX = self->speed;

		self->x += target > self->x ? self->dirX : -self->dirX;

		if (self->x < self->startX)
		{
			self->x = self->startX;

			/* Throw if at the edge of the screen */

			self->thinkTime = 15;

			self->dirX = 0;

			if (player.y < getMapStartY() + SCREEN_HEIGHT)
			{
				self->action = &bridgeDestroy;
			}
		}

		else if (self->x > self->endX)
		{
			self->x = self->endX;

			/* Throw if at the edge of the screen */

			self->thinkTime = 15;

			self->dirX = 0;

			if (player.y < getMapStartY() + SCREEN_HEIGHT)
			{
				self->action = &bridgeDestroy;
			}
		}
	}
}

static void bridgeDestroy()
{
	int currentFrame;
	float frameTimer;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		currentFrame = self->currentFrame;
		frameTimer = self->frameTimer;

		setEntityAnimation(self, "LANCE_THROW");

		self->currentFrame = currentFrame;
		self->frameTimer = frameTimer;

		playSoundToMap("sound/boss/gargoyle/gargoyle_lance_stab.ogg", -1, self->x, self->y, 0);

		self->target->mental = -2;

		self->action = &bridgeDestroyWait;
	}

	checkToMap(self);
}

static void bridgeDestroyWait()
{
	if (self->target->mental == 0)
	{
		setEntityAnimation(self, "LANCE_THROW_READY");

		self->mental--;

		self->thinkTime = 60;

		self->action = self->mental > 0 ? &bridgeDestroyFollowPlayer : &bridgeDestroyFinish;
	}

	checkToMap(self);
}

static void bridgeDestroyFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void lanceThrowInit()
{
	Target *t = getTargetByName("GARGOYLE_MID_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Gargoyle cannot find target");
	}

	self->face = RIGHT;

	setEntityAnimation(self, "LANCE_THROW_READY");

	self->targetX = self->x;
	self->targetY = t->y;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= self->speed;
	self->dirY *= self->speed;

	self->flags |= FLY;

	self->action = &lanceThrowMoveToTarget;

	self->thinkTime = 30;

	checkToMap(self);
}

static void lanceThrowMoveToTarget()
{
	if (atTarget())
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->targetX = getMapStartX() + prand() % (SCREEN_WIDTH - self->w);
			self->targetY = self->y;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= 3;
			self->dirY *= 3;

			self->thinkTime = 60;

			self->action = &lanceThrow;
		}
	}

	checkToMap(self);
}

static void lanceThrow()
{
	int currentFrame;
	float frameTimer;

	if (atTarget())
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			currentFrame = self->currentFrame;
			frameTimer = self->frameTimer;

			setEntityAnimation(self, "LANCE_THROW");

			self->currentFrame = currentFrame;
			self->frameTimer = frameTimer;

			playSoundToMap("sound/boss/gargoyle/gargoyle_lance_stab.ogg", -1, self->x, self->y, 0);

			self->target->mental = -1;

			self->target->dirY = 4;

			self->action = &lanceThrowWait;

			self->thinkTime = 30;

			self->maxThinkTime++;
		}
	}

	checkToMap(self);
}

static void lanceThrowWait()
{
	if (self->target->inUse == FALSE)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &attackFinished;
		}
	}

	checkToMap(self);
}

static void createLanceInit()
{
	self->flags &= ~FLY;

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		self->thinkTime = 30;

		self->action = &createLance;
	}

	checkToMap(self);
}

static void createLance()
{
	Entity *e;

	setEntityAnimation(self, "CREATE_LANCE");

	self->face = RIGHT;

	self->mental = 1;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		playSoundToMap("sound/boss/gargoyle/gargoyle_create_lance.ogg", -1, self->x, self->y, 0);

		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Gargoyle's Lance");
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

		e->maxThinkTime = self->maxThinkTime;

		self->action = &createLanceWait;

		self->thinkTime = 90;
	}

	checkToMap(self);
}

static void createLanceWait()
{
	if (self->mental == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->health = self->maxHealth;

			facePlayer();

			setEntityAnimation(self, "STAND");

			self->action = &attackFinished;
		}
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

	if (self->head->flags & FLASH)
	{
		self->flags |= FLASH;
	}

	else
	{
		self->flags &= ~FLASH;
	}

	if (self->head->flags & NO_DRAW)
	{
		self->flags |= NO_DRAW;
	}

	else
	{
		self->flags &= ~NO_DRAW;
	}

	if (self->mental == -1)
	{
		self->dirY = 14;

		setEntityAnimation(self, "LANCE_THROW");

		self->pain = &enemyPain;

		self->touch = &entityTouch;

		self->action = &lanceDrop;
	}

	else if (self->mental == -2)
	{
		self->dirY = 14;

		setEntityAnimation(self, "LANCE_THROW");

		self->flags |= ATTACKING;

		self->touch = &entityTouch;

		self->fallout = &lanceFallout;

		self->action = &lanceDestroyBridge;
	}

	self->alpha = self->head->alpha;
}

static void lanceDestroyBridge()
{
	checkToMap(self);

	if (self->y > getMapStartY() + SCREEN_HEIGHT)
	{
		lanceFallout();
	}
}

static void lanceFallout()
{
	int distance;
	float checkpointX, checkpointY, x;
	int startX, endX;
	EntityList *el, *entities;

	entities = getEntities();

	distance = 0;

	startX = getMapStartX();
	endX   = getMapStartX() + SCREEN_WIDTH;
	
	checkpointX = startX;

	/* Get a piece furthest away from the boss */

	for (el=entities->next;el!=NULL;el=el->next)
	{
		if (el->entity->inUse == TRUE && el->entity->type == WEAK_WALL && el->entity->mental == -1 &&
			el->entity->touch != NULL && el->entity->x >= startX && el->entity->x < endX)
		{
			if (abs(el->entity->x - self->head->x) > distance)
			{
				distance = abs(el->entity->x - self->head->x);

				checkpointX = el->entity->x;
			}
		}
	}

	getCheckpoint(&x, &checkpointY);

	setCheckpoint(checkpointX, checkpointY);

	self->currentFrame = self->head->currentFrame;
	self->frameTimer = self->head->frameTimer;

	self->y = self->head->y + self->offsetY;

	self->action = &lanceAttackTeleportFinish;
}

static void lanceDrop()
{
	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		self->takeDamage = &entityTakeDamageNoFlinch;

		self->die = &lanceDie;

		setEntityAnimation(self, "LANCE_IN_GROUND");

		playSoundToMap("sound/enemy/ground_spear/spear.ogg", -1, self->x, self->y, 0);

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

	checkToMap(self);
}

static void lanceDie()
{
	int i;
	Entity *e;
	char name[MAX_VALUE_LENGTH];

	playSoundToMap("sound/enemy/centurion/centurion_die.ogg", -1, self->x, self->y, 0);

	snprintf(name, sizeof(name), "%s_piece", self->name);

	for (i=0;i<6;i++)
	{
		e = addTemporaryItem(name, self->x, self->y, self->face, 0, 0);

		e->x += (self->w - e->w) / 2;
		e->y += (self->w - e->w) / 2;

		e->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->thinkTime = 60 + (prand() % 60);
	}

	self->inUse = FALSE;
}

static void petrifyAttackInit()
{
	self->flags &= ~FLY;

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime = 30;

		self->action = &petrifyAttack;

		self->mental = 0;
	}

	checkToMap(self);
}

static void petrifyAttack()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental == 0)
		{
			setEntityAnimation(self, "CREATE_LANCE");

			self->thinkTime = 60;

			self->mental = 1;
		}

		else
		{
			playSoundToMap("sound/boss/gargoyle/gargoyle_petrify.ogg", -1, self->x, self->y, 0);

			fadeFromColour(255, 255, 0, 30);

			if (player.element == SLIME)
			{
				player.die();
			}

			else
			{
				setPlayerPetrified();
			}

			self->thinkTime = 300;

			self->action = &petrifyAttackWait;
		}
	}

	checkToMap(self);
}

static void petrifyAttackWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void weaponRemoveBlastInit()
{
	self->flags &= ~FLY;

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime = 30;

		self->action = &weaponRemoveBlast;

		self->mental = 1 + prand() % 3;
	}

	checkToMap(self);

	cloneCheck();
}

static void weaponRemoveBlast()
{
	Entity *e;

	facePlayer();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "WEAPON_REMOVE");

		playSoundToMap("sound/boss/snake_boss/snake_boss_shot.ogg", -1, self->x, self->y, 0);

		e = addProjectile("boss/gargoyle_weapon_remove_blast", self, self->x, self->y, self->face == LEFT ? -8 : 8, 0);

		e->face = self->face;

		e->damage = 0;

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

		self->action = &weaponRemoveBlastFinish;

		self->mental--;

		self->thinkTime = self->mental > 0 ? 30 : 60;
	}

	checkToMap(self);

	cloneCheck();
}

static void blastRemoveWeapon(Entity *other)
{
	Entity *e;

	if (other->type == PLAYER && !(other->flags & INVULNERABLE))
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

			e->flags |= LIMIT_TO_SCREEN;
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

			e->flags |= LIMIT_TO_SCREEN;
		}

		playSoundToMap("sound/common/punch.ogg", EDGAR_CHANNEL, self->x, self->y, 0);

		setCustomAction(other, &invulnerable, 60, 0, 0);

		setPlayerStunned(30);

		other->x -= other->dirX;
		other->y -= other->dirY;

		other->dirX = (6 + prand() % 3) * (self->dirX < 0 ? -1 : 1);
		other->dirY = -8;

		self->inUse = FALSE;
	}
}

static void weaponRemoveBlastFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = self->mental > 0 ? &weaponRemoveBlast : &attackFinished;
	}

	checkToMap(self);

	cloneCheck();
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

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

			/* Don't die if you've still got lances to wield */

			if (self->maxThinkTime < 3 && self->health < self->maxHealth / 4)
			{
				self->health = self->maxHealth / 4;
			}
		}

		else
		{
			self->health = 0;

			if (self->maxThinkTime >= 3)
			{
				self->damage = 0;

				self->die();
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
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental = 2;

		self->endX = 1;

		self->action = &createLightningOrb;
	}

	checkToMap(self);
}

static void lanceAttack2()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->head->action = &becomeMiniGargoyleInit;

		self->action = &lanceAttack2Wait;
	}

	checkToMap(self);
}

static void lanceAttack2Wait()
{
	checkToMap(self);
}

static void lanceAttack3()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime = 0;

		self->action = &fakeLanceDropInit;
	}

	checkToMap(self);
}

static void fakeLanceDropInit()
{
	Target *t = getTargetByName("GARGOYLE_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Lance cannot find target");
	}

	self->flags |= (FLY|NO_DRAW);

	setEntityAnimation(self, "LANCE_THROW");

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

	self->y = t->y;

	self->thinkTime = 30;

	self->action = &fakeLanceDropAppear;
}

static void fakeLanceDropAppear()
{
	int i, real, x, lanceCount;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		lanceCount = 5;

		real = prand() % lanceCount;

		x = getMapStartX() + SCREEN_WIDTH / (lanceCount + 1);

		for (i=0;i<lanceCount;i++)
		{
			if (i == real)
			{
				e = self;
			}

			else
			{
				e = getFreeEntity();

				if (e == NULL)
				{
					showErrorAndExit("No free slots to add a fake lance");
				}

				loadProperties("boss/gargoyle_fake_lance", e);

				e->draw = &drawLoopingAnimationToMap;
				e->touch = &entityTouch;
				e->takeDamage = &entityTakeDamageNoFlinch;
				e->die = &fakeLanceDie;
				e->pain = &enemyPain;

				e->type = ENEMY;

				setEntityAnimation(e, "LANCE_THROW");

				e->mental = 0;

				e->head = self;
			}

			e->action = &fakeLanceDropWait;

			e->flags |= (FLY|NO_DRAW);

			e->x = x;
			e->y = self->y;

			e->thinkTime = 30 * i;

			e->maxThinkTime = 30 * (lanceCount - i);

			x += SCREEN_WIDTH / (lanceCount + 1);
		}

		self->endX = lanceCount - 1;
	}
}

static void fakeLanceDropWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~NO_DRAW;

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

		self->action = &fakeLanceDrop;

		self->thinkTime = self->maxThinkTime;
	}

	checkToMap(self);
}

static void fakeLanceDrop()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirY = 14;
	}

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		setEntityAnimation(self, "LANCE_IN_GROUND");

		if (self->mental == -1)
		{
			self->thinkTime = 600;

			playSoundToMap("sound/enemy/ground_spear/spear.ogg", -1, self->x, self->y, 0);
		}

		else
		{
			self->thinkTime = 180 + prand() % 120;
		}

		self->action = &fakeLanceDropExplodeWait;
	}

	checkToMap(self);
}

static void fakeLanceDropExplodeWait()
{
	if (self->mental != -1)
	{
		self->thinkTime--;

		if (self->thinkTime < 120)
		{
			if (self->thinkTime % 3 == 0)
			{
				self->flags ^= FLASH;
			}
		}

		if (self->thinkTime <= 0)
		{
			self->action = &lanceExplode;
		}
	}

	else if (self->endX <= 0)
	{
		self->action = &fakeLanceDropInit;
	}

	checkToMap(self);
}

static void lanceExplode()
{
	int x, y;
	Entity *e;

	e = addProjectile("common/green_blob", self->head, 0, 0, -6, 0);

	x = self->x + self->w / 2 - e->w / 2;
	y = self->y + self->h / 2 - e->h / 2;

	e->x = x;
	e->y = y;

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self->head, x, y, 6, 0);

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	playSoundToMap("sound/common/explosion.ogg", -1, self->x, self->y, 0);

	self->head->endX--;

	self->inUse = FALSE;
}

static void fakeLanceDie()
{
	self->head->endX--;

	entityDieNoDrop();
}

static void createLightningOrb()
{
	Entity *e;
	Target *t;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a lightning orb");
		}

		t = getTargetByName("GARGOYLE_TOP_TARGET");

		if (t == NULL)
		{
			showErrorAndExit("Gargoyle cannot find target");
		}

		loadProperties("boss/gargoyle_lightning_orb", e);

		e->x = self->x;
		e->y = self->y;

		e->startX = getMapStartX();
		e->endX   = getMapStartX() + SCREEN_WIDTH - e->w;

		e->targetY = t->y;

		e->endY = self->y + self->h;

		e->action = &orbMoveToTop;

		e->draw = &drawLoopingAnimationToMap;
		e->touch = NULL;
		e->takeDamage = NULL;

		e->type = ENEMY;

		e->head = self;

		e->mental = self->endX;

		setEntityAnimation(e, "STAND");

		self->mental--;

		self->endX++;

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
	checkToMap(self);
}

static void orbMoveToTop()
{
	self->y -= self->speed * 2;

	if (self->y <= self->targetY)
	{
		self->startY = self->targetY;

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

		self->action = &orbCastLightning2;
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

			self->action = &orbCastLightning2;
		}

		else if (self->x > self->endX)
		{
			self->x = self->endX;

			self->thinkTime = 30;

			self->dirX = 0;

			self->action = &orbCastLightning2;
		}
	}

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static void orbCastLightning2()
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

		self->action = &orbCastLightningFinish2;

		self->thinkTime = 30 * self->mental;
	}
}

static void orbCastLightningFinish2()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &orbFollowPlayer;

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

static void becomeMiniGargoyleInit()
{
	int i;
	Entity *e;

	fadeFromColour(255, 255, 255, 30);

	self->flags |= NO_DRAW;

	self->touch = NULL;

	self->mental = 0;

	self->endX = 0;

	for (i=0;i<18;i++)
	{
		e = addEnemy("boss/mini_gargoyle", 0, 0);

		setEntityAnimationByID(e, i);

		e->x = self->x + e->offsetX;
		e->y = self->y + e->offsetY;

		e->endX = e->x;
		e->endY = e->y;

		e->flags |= LIMIT_TO_SCREEN;

		setEntityAnimation(e, "STAND");

		e->targetY = getMapStartY() - player.h - TILE_SIZE;

		e->head = self;

		e->thinkTime = 60 + prand() % 120;

		self->mental++;

		self->endX++;
	}

	self->action = &becomeMiniGargoyleWait;

	checkToMap(self);
}

static void becomeMiniGargoyleWait()
{
	if (self->target->inUse == FALSE)
	{
		self->action = &becomeMiniGargoyleFinish;
	}

	checkToMap(self);
}

static void becomeMiniGargoyleFinish()
{
	if (self->endX <= 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->flags & NO_DRAW)
			{
				fadeFromColour(255, 255, 255, 30);

				self->flags &= ~NO_DRAW;

				self->thinkTime = 60;
			}

			else
			{
				self->touch = &entityTouch;

				self->action = &attackFinished;
			}
		}
	}

	checkToMap(self);
}

static void lanceAttackTeleportFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~(NO_DRAW|ATTACKING);

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

		self->touch = NULL;

		self->action = &lanceWait;

		self->mental = 0;
	}
}

static void addStoneCoat()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Gargoyle Stone Coat");
	}

	loadProperties("boss/gargoyle_stone_coat", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &coatWait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->takeDamage = NULL;

	e->type = ENEMY;

	e->head = self;

	self->target = e;

	setEntityAnimation(e, getAnimationTypeAtIndex(self));
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
			self->head->mental = 2;

			self->inUse = FALSE;
		}
	}

	else if (self->head->mental == 2)
	{
		self->alpha++;

		if (self->alpha >= 255)
		{
			self->head->mental = 0;

			self->alpha = 255;
		}
	}

	else if (self->head->mental == -1)
	{
		self->inUse = FALSE;
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
	int i;
	long onGround;
	Entity *e;

	self->action = &die;

	self->damage = 0;

	self->takeDamage = NULL;

	setEntityAnimation(self, "FACE_FRONT_CROUCH");

	self->dirX = 0;

	self->flags &= ~(FLY|LIMIT_TO_SCREEN);

	onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (landedOnGround(onGround) == TRUE)
	{
		playSoundToMap("sound/enemy/red_grub/thud.ogg", -1, self->x, self->y, 0);

		shakeScreen(MEDIUM, 30);

		for (i=0;i<30;i++)
		{
			e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

			if (e != NULL)
			{
				e->y -= prand() % e->h;
			}
		}
	}

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		self->maxThinkTime = 0;

		self->health = 0;

		self->thinkTime = 180;

		self->action = &dieWait;

		self->startX = self->x;
	}
}

static void dieWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		switch (self->maxThinkTime)
		{
			case 0:
				setEntityAnimation(self, "FACE_FRONT");

				self->thinkTime = 60;

				self->maxThinkTime = 1;
			break;

			case 1:
				facePlayer();

				setEntityAnimation(self, "FACE_PLAYER");

				self->thinkTime = 60;

				self->maxThinkTime = 2;
			break;

			case 2:
				setEntityAnimation(self, "STAND");

				self->thinkTime = 60;

				self->maxThinkTime = 3;
			break;

			case 3:
				setEntityAnimation(self, "REACH");

				self->thinkTime = 30;

				self->maxThinkTime = 4;
			break;

			case 4:
				addStoneCoat();

				self->target->alpha = 0;

				self->mental = 2;

				self->maxThinkTime = 5;

				playSoundToMap("sound/boss/gargoyle/gargoyle_stone_to_flesh.ogg", BOSS_CHANNEL, self->x, self->y, -1);
			break;

			case 5:
				self->x = self->startX + 1 * (prand() % 2 == 0 ? 1 : -1);

				if (self->mental == 0)
				{
					stopSound(BOSS_CHANNEL);

					self->x = self->startX;

					self->mental = -1;

					setEntityAnimation(self, "REACH_STONE");

					self->thinkTime = 30;

					self->maxThinkTime = 6;
				}
			break;

			default:
				self->takeDamage = &stoneTakeDamage;

				self->touch = &stoneTouch;

				self->activate = &activate;

				self->health = 8;

				self->action = &dieFinish;

				clearContinuePoint();

				increaseKillCount();

				freeBossHealthBar();

				fadeBossMusic();

				fireTrigger(self->objectiveName);

				fireGlobalTrigger(self->objectiveName);
			break;
		}
	}
}

static void stoneTouch(Entity *other)
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

static void stoneTakeDamage(Entity *other, int damage)
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

	for (i=0;i<7;i++)
	{
		e = addTemporaryItem("boss/gargoyle_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += self->w / 2 - e->w / 2;
		e->y += self->h / 2 - e->h / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->thinkTime = 60 + (prand() % 60);
	}

	playSoundToMap("sound/common/crumble.ogg", BOSS_CHANNEL, self->x, self->y, 0);

	self->inUse = FALSE;
}

static void addExitTrigger(Entity *e)
{
	char itemName[MAX_LINE_LENGTH];

	snprintf(itemName, MAX_LINE_LENGTH, "\"%s\" 1 UPDATE_EXIT \"GARGOYLE_BOSS\"", e->objectiveName);

	addGlobalTriggerFromScript(itemName);
}

static void cloneCheck()
{
	if (self->head != NULL && self->head->health <= 0)
	{
		self->action = &cloneDie;
	}
}

static void cloneDie()
{
	self->action = &cloneDie;

	self->frameSpeed = 0;

	self->damage = 0;

	self->dirX = 0;
	self->dirY = 0;

	self->takeDamage = NULL;

	self->alpha -= 2;

	if (self->alpha <= 0)
	{
		self->inUse = FALSE;
	}
}

static void creditsMove()
{
	if (self->mental == 0)
	{
		addLance();

		self->mental = 1;
	}

	setEntityAnimation(self, "STAND");

	self->creditsAction = &bossMoveToMiddle;
}

static void addLance()
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Gargoyle's Lance");
	}

	loadProperties("boss/gargoyle_lance_1", e);

	e->creditsAction = &lanceCreditsMove;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->head = self;

	setEntityAnimation(e, "STAND");
}

static void lanceCreditsMove()
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

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static void fallout()
{
	Target *t = getTargetByName("GARGOYLE_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Gargoyle cannot find target");
	}

	setEntityAnimation(self, "FLY");

	self->x = t->x;
	self->y = t->y;

	self->dirX = 0;
	self->dirY = 0;

	self->flags |= FLY|NO_DRAW;

	self->thinkTime = 60;

	self->action = &falloutWait;
}

static void falloutWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~NO_DRAW;

		self->thinkTime = 30;

		self->action = &entityWait;

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		playSoundToMap("sound/common/teleport.ogg", BOSS_CHANNEL, self->x, self->y, 0);
	}
}
