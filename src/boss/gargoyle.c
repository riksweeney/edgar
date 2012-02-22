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
#include "../event/script.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../inventory.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../hud.h"
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
static void petrifyPlayer(void);
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

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "CROUCH");

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case -1:
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
		self->maxThinkTime = 0;

		playDefaultBossMusic();

		self->mental = 2;

		setContinuePoint(FALSE, self->name, NULL);

		initBossHealthBar();

		self->takeDamage = &takeDamage;

		self->action = &entityWait;

		self->thinkTime = 90;

		self->flags |= LIMIT_TO_SCREEN;
	}
}

static void attackFinished()
{
	Target *t = getTargetByName("GARGOYLE_LEFT_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Gargoyle cannot find target");
	}

	setEntityAnimation(self, "FLY");

	self->flags |= FLY;

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

		self->action = &entityWait;
	}

	facePlayer();

	checkToMap(self);
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
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
								self->action = &lightningGridAttackInit;
							break;
						}

						self->action = &weaponRemoveBlastInit;
					}
				break;

				case 1:
					if (self->health == self->maxHealth / 4)
					{
						self->action = &lanceThrowInit;
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
				break;

				default:
					if (self->target->inUse == FALSE && self->maxThinkTime == 4)
					{
						self->maxThinkTime = 5;

						self->action = &splitInHalfInit;
					}

					else
					{
						switch (prand() % 3)
						{
							case 0:
								self->action = &dropAttackInit;
							break;

							case 1:
								self->action = &weaponRemoveBlastInit;
							break;

							default:
								self->action = &attackFinished;
							break;
						}
					}
				break;
			}
		}
	}

	checkToMap(self);
}

static void splitInHalfInit()
{
	setEntityAnimation(self, "FACE_FRONT");

	self->thinkTime = 30;

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

		e->action = &splitInHalfMove;

		e->draw = &drawLoopingAnimationToMap;

		e->touch = &entityTouch;

		e->takeDamage = &entityTakeDamageNoFlinch;

		e->die = &entityDieNoDrop;

		e->type = ENEMY;

		e->head = self;

		e->flags |= LIMIT_TO_SCREEN|DO_NOT_PERSIST;

		e->targetX = self->x - self->w;

		e->targetY = self->y;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->maxThinkTime = self->maxThinkTime;

		e->thinkTime = 0;

		self->health = self->maxHealth;

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

	if (t != NULL)
	{
		showErrorAndExit("Gargoyle cannot find target");
	}

	self->targetX = self->x;
	self->targetY = t->y;

	self->flags |= FLY;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->action = &dropAttackMoveToTop;

	checkToMap(self);

	cloneCheck();
}

static void dropAttackMoveToTop()
{
	if (atTarget())
	{
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

				self->action = &dropAttack;
			}
		}
	}

	checkToMap(self);

	cloneCheck();
}

static void dropAttack()
{
	int i;
	long onGround = self->flags & ON_GROUND;
	Entity *e;

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
	}

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &attackFinished;
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

		self->endX = 0;

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

			self->endX++;
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
			self->head->endX--;

			self->mental = 0;
		}

		else if (self->head->endX <= 0)
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

		calculatePath(self->x, self->y, player.x, player.y + player.h - self->h, &self->dirX, &self->dirY);

		self->dirX *= 16;
		self->dirY *= 16;

		self->action = &lanceStab;

		self->reactToBlock = &lanceStabReactToBlock;
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
	self->flags &= ~FLY;

	self->dirX = self->face == RIGHT ? -5 : 5;
	self->dirY = -6;

	self->action = &lanceStabFinish;

	self->thinkTime = 30;
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

	self->alpha -= 2;

	if (self->alpha <= 0)
	{
		t = getTargetByName("GARGOYLE_TOP_TARGET");

		if (t == NULL)
		{
			showErrorAndExit("Gargoyle cannot find target");
		}

		self->alpha = 0;

		self->targetX = self->x;
		self->targetY = t->y;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed;
		self->dirY *= self->speed;

		self->flags |= FLY;

		self->action = &invisibleAttackMoveToTop;

		checkToMap(self);
	}
}

static void invisibleAttackMoveToTop()
{
	if (atTarget())
	{
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

	checkToMap(self);
}

static void invisibleDrop()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->standingOn != NULL || (self->flags & ON_GROUND))
		{
			self->alpha = 255;

			self->thinkTime = 30;

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
		self->mental--;

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

			self->alpha = 0;

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

	self->action = &bridgeDestroyMoveToTarget;

	checkToMap(self);
}

static void bridgeDestroyMoveToTarget()
{
	if (atTarget())
	{
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

		self->thinkTime = 30;

		self->dirX = 0;

		self->action = &bridgeDestroy;
	}

	else
	{
		self->dirX = self->speed * 1.5;

		self->x += target > self->x ? self->dirX : -self->dirX;

		if (self->x < self->startX)
		{
			self->x = self->startX;

			/* Throw if at the edge of the screen */

			if (self->x == getMapStartX())
			{
				self->thinkTime = 30;

				self->dirX = 0;

				self->action = &bridgeDestroy;
			}
		}

		else if (self->x > self->endX)
		{
			self->x = self->endX;

			/* Throw if at the edge of the screen */

			if (self->x == getMapStartX() + SCREEN_WIDTH - self->w)
			{
				self->thinkTime = 30;

				self->dirX = 0;

				self->action = &bridgeDestroy;
			}
		}
	}

	checkToMap(self);
}

static void bridgeDestroy()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->target->mental = -2;

		self->action = &bridgeDestroyWait;
	}

	checkToMap(self);
}

static void bridgeDestroyWait()
{
	if (self->target->mental == 0)
	{
		self->mental--;

		self->action = self->mental > 0 ? &bridgeDestroyFollowPlayer : &attackFinished;
	}
}

static void lanceThrowInit()
{
	Target *t = getTargetByName("GARGOYLE_LEFT_TARGET");

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

		e->maxThinkTime = 0;

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
		self->action = &lanceDrop;
	}

	else if (self->mental == -2)
	{
		self->flags |= ATTACKING;

		self->fallout = &lanceFallout;

		self->action = &lanceDestroyBridge;
	}
}

static void lanceDestroyBridge()
{
	self->dirY = 8;

	checkToMap(self);
}

static void lanceFallout()
{
	self->y = self->head->y + self->offsetY;

	self->action = &lanceAttackTeleportFinish;
}

static void lanceDrop()
{
	self->flags &= ~FLY;

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
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

static void petrifyAttackInit()
{
	self->flags &= ~FLY;

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		self->thinkTime = 120;

		self->action = &petrifyAttack;
	}

	checkToMap(self);
}

static void petrifyAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a petrification");
		}

		loadProperties("boss/gargoyle_lightning_orb", e);

		e->x = self->x;
		e->y = self->y;

		e->startX = e->x;

		e->action = &petrifyPlayer;

		e->draw = &drawLoopingAnimationToMap;
		e->touch = NULL;
		e->takeDamage = NULL;

		e->type = ENEMY;

		e->head = self;

		setEntityAnimation(e, "STAND");

		self->mental = 1;

		self->thinkTime = 120;

		self->action = &petrifyAttackWait;

		setPlayerLocked(TRUE);
	}

	checkToMap(self);
}

static void petrifyAttackWait()
{
	if (self->mental == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &attackFinished;
		}
	}

	checkToMap(self);
}

static void petrifyPlayer()
{
	int i;
	Entity *e;

	self->thinkTime--;

	setInfoBoxMessage(0, 255, 255, 255, _("Press buttons to break the petrification!"));

	if (self->thinkTime <= 0 && self->mental < 4)
	{
		self->mental++;

		if (self->mental >= 4)
		{
			self->mental = 4;

			if (player.health > 0)
			{
				removeInventoryItemByObjectiveName("Amulet of Resurrection");

				player.die();

				player.flags |= NO_DRAW;
			}
		}

		setEntityAnimationByID(self, self->mental);

		self->thinkTime = 60;
	}

	if (input.up == 1 || input.down == 1 || input.right == 1 || input.left == 1 ||
		input.previous == 1 || input.next == 1 || input.jump == 1 ||
		input.activate == 1 || input.attack == 1 || input.interact == 1 || input.block == 1)
	{
		self->health--;

		input.up = 0;
		input.down = 0;
		input.right = 0;
		input.left = 0;
		input.previous = 0;
		input.next = 0;
		input.jump = 0;
		input.activate = 0;
		input.attack = 0;
		input.interact = 0;
		input.block = 0;

		e = addTemporaryItem("misc/petrify_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += self->w / 2 - e->w / 2;
		e->y += self->h / 2 - e->h / 2;

		e->dirX = (prand() % 6) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = -4;

		e->thinkTime = 60 + (prand() % 120);

		if (self->health <= 0)
		{
			for (i=0;i<8;i++)
			{
				e = addTemporaryItem("misc/petrify_piece", self->x, self->y, RIGHT, 0, 0);

				e->x += prand() % self->w;
				e->y += prand() % self->h;

				e->dirX = (prand() % 4) * (prand() % 2 == 0 ? -1 : 1);
				e->dirY = -2.5;

				e->thinkTime = 60 + (prand() % 120);
			}

			self->head->mental = 0;

			self->inUse = FALSE;

			setPlayerLocked(FALSE);
		}
	}
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

		self->thinkTime = self->mental > 0 ? 15 : 60;
	}

	checkToMap(self);

	cloneCheck();
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
		self->maxThinkTime = 0;

		self->mental = 2;

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

				e->type = ENEMY;

				setEntityAnimation(e, "STAND");

				e->mental = -1;

				e->head = self->head;
			}

			e->action = &fakeLanceDropWait;

			e->flags &= ~NO_DRAW;

			e->flags |= FLY;

			e->x = x;
			e->y = self->y;

			e->thinkTime = 30 * i;

			e->maxThinkTime = (30 * lanceCount) + 60;

			x += SCREEN_WIDTH / (lanceCount + 1);
		}
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
		self->flags &= ~FLY;
	}

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		if (self->mental != -1)
		{
			self->thinkTime = 360;

			playSoundToMap("sound/enemy/ground_spear/spear.ogg", -1, self->x, self->y, 0);
		}

		else
		{
			self->thinkTime = 240 + prand() % 120;
		}

		self->action = &fakeLanceDropExplodeWait;
	}

	checkToMap(self);
}

static void fakeLanceDropExplodeWait()
{
	self->thinkTime--;

	if (self->mental == -1 && self->thinkTime < 120)
	{
		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= FLASH;
		}
	}

	if (self->thinkTime <= 0)
	{
		self->action = self->mental == -1 ? &lanceExplode : &fakeLanceDropInit;
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

	e = addProjectile("common/green_blob", self->head, x, y, -6, -6);

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self->head, x, y, 0, -6);

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self->head, x, y, 6, -6);

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self->head, x, y, -6, 6);

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self->head, x, y, 0, 6);

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self->head, x, y, 6, 6);

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self->head, x, y, 6, 0);

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	playSoundToMap("sound/common/explosion.ogg", -1, self->x, self->y, 0);

	self->inUse = FALSE;
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

		e->endY = self->y + self->h;

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
	checkToMap(self);
}

static void orbMoveToTop()
{
	self->y -= self->speed;

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

		self->thinkTime = 30;
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
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~NO_DRAW;

		self->touch = &entityTouch;

		self->action = &attackFinished;
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

	loadProperties("boss/gargoyle_stone_coat", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &coatWait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;

	e->head = self;

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
	int i;
	long onGround;
	Entity *e;

	self->dirX = 0;

	self->flags &= ~FLY;

	onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (landedOnGround(onGround) == TRUE)
	{
		playSoundToMap("sound/common/crash.ogg", BOSS_CHANNEL, self->x, self->y, 0);

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
		self->damage = 0;

		self->takeDamage = NULL;

		addStoneCoat();

		self->health = 0;

		self->thinkTime = 60;

		self->action = &dieWait;

		self->startX = self->x;
	}
}

static void dieWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "REACH");

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

			setEntityAnimation(self, "REACH_STONE");

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

static void addExitTrigger(Entity *e)
{
	char itemName[MAX_LINE_LENGTH];

	snprintf(itemName, MAX_LINE_LENGTH, "\"%s\" 1 UPDATE_EXIT \"GARGOYLE_BOSS\"", e->objectiveName);

	addGlobalTriggerFromScript(itemName);
}

static void cloneCheck()
{
	if (self->head != NULL && self->head->health > 0)
	{
		self->takeDamage = NULL;

		self->dirX = 0;
		self->dirY = 0;

		self->action = &cloneDie;
	}
}

static void cloneDie()
{
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

	e->action = &lanceCreditsMove;

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
