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
#include "../enemy/enemies.h"
#include "../enemy/rock.h"
#include "../entity.h"
#include "../event/global_trigger.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/graphics.h"
#include "../hud.h"
#include "../item/item.h"
#include "../map.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/target.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void blackBookAttackFinished(void);
static void blackBookWait(void);
static void hover(void);
static void transformWait(void);
static void transformRemove(void);
static int energyBarDraw(void);
static void energyBarWait(void);
static void initEnergyBar(Entity *);
static void addSmokeAlongBody(void);
static void blackBookDie(void);
static void blackBookShudder(void);
static void addBack(void);
static void backWait(void);
static void dieFinish(void);
static void creditsMove(void);
static void addExitTrigger(Entity *);

static void becomeKingGrub(void);
static void kingGrubWait(void);
static void kingGrubSpitStart(void);
static void kingGrubSpit(void);
static void kingGrubSpitEnd(void);
static void kingGrubTakeDamage(Entity *, int);
static void kingGrubDie(void);
static void kingGrubAttackFinished(void);
static void kingGrubSpinAttackStart(void);
static void kingGrubSpinAttack(void);
static void kingGrubSpinAttackEnd(void);
static void kingGrubBounceAttackStart(void);
static void kingGrubBounceAttack(void);
static void kingGrubBounceAttackEnd(void);
static void kingGrubShudder(void);

static void becomeQueenWasp(void);
static void queenWaspTakeDamage(Entity *, int);
static void queenWaspWait(void);
static void queenWaspHover(void);
static void queenWaspMoveToTarget(void);
static void queenWaspFlyToTopTarget(void);
static void queenWaspBulletFireInit(void);
static void queenWaspBulletFireMoveToPosition(void);
static void queenWaspFireBullets(void);
static void queenWaspSlimeFireInit(void);
static void queenWaspSlimeFireMoveToPosition(void);
static void queenWaspFireSlime(void);
static void queenWaspAttackFinished(void);
static void queenWaspSlimePlayer(Entity *);
static void queenWaspHeadButtInit(void);
static void queenWaspHeadButtMoveToPosition(void);
static void queenWaspMoveToHeadButtRange(void);
static void queenWaspHeadButt(void);
static void queenWaspHeadButtFinish(void);
static void queenWaspSelectRandomBottomTarget(void);
static void queenWaspReactToHeadButtBlock(Entity *);
static void queenWaspDropInit(void);
static void queenWaspDrop(void);
static void queenWaspDropWait(void);
static void queenWaspRamTouch(Entity *);
static void queenWaspDie(void);
static void queenWaspShudder(void);

static void becomeBlob(void);
static void blobBounceAroundInit(void);
static void blobBounceAround(void);
static void blobPunchAttackInit(void);
static void blobPunchSink(void);
static void blobLookForPlayer(void);
static void blobPunch(void);
static void blobPunchFinish(void);
static void blobShudder(void);
static void blobGrubAttackInit(void);
static void blobGrubAttackWait(void);
static void blobSpinAttackStart(void);
static void blobSpinAttack(void);
static void blobSpinAttackEnd(void);
static void blobGrubAttackFinish(void);
static void blobAttackFinished(void);
static void blobTakeDamage(Entity *, int);
static void blobDie(void);
static void blobShudder(void);

static void becomeAwesome(void);
static void awesomeIntro(void);
static void awesomeIntroWait(void);
static void awesomeWait(void);
static void awesomeAttackFinished(void);
static void awesomeTeleportIn(void);
static void awesomeTeleportOut(void);
static void awesomeTeleportWait(void);
static void awesomeTakeDamage(Entity *, int);
static void awesomeDie(void);
static void awesomeHealPartner(void);
static void awesomeAddStunStar(void);
static void awesomeStarWait(void);
static void awesomeFireballAttackInit(void);
static void awesomeFireballAttack(void);
static void awesomeFireballAttackFinished(void);
static void awesomeSuperFireballAttackInit(void);
static void awesomeSuperFireballAttack(void);
static void awesomeSuperFireballAttackFinished(void);
static void awesomeFireballTouch(Entity *);
static void awesomeDropAttack(void);
static void awesomeDropAttackInit(void);
static void awesomeSuperDropAttack(void);
static void awesomeSuperDropAttackInit(void);
static void awesomeSuperDropAttackFinished(void);
static void awesomeFireballChargeWait(void);
static void awesomeSuperSpearAttackInit(void);
static void awesomeSuperSpearAttack(void);
static void awesomeSuperSpearAttackFinished(void);
static void awesomeSpearWait(void);
static void awesomeSpearRise(void);
static void awesomeSpearSink(void);
static void awesomeFireballMove(void);
static void awesomeMeterDie(void);

static void becomeGargoyle(void);
static void gargoyleInitialise(void);
static void gargoyleAttackFinished(void);
static void gargoyleAttackFinishedMoveUp(void);
static void gargoyleAttackFinishedMoveHorizontal(void);
static void gargoyleWait(void);
static void gargoyleSplitInHalfInit(void);
static void gargoyleSplitInHalf(void);
static void gargoyleSplitInHalfMove(void);
static void gargoyleDropAttackInit(void);
static void gargoyleDropAttackMoveToTop(void);
static void gargoyleDropAttackFollowPlayer(void);
static void gargoyleDropAttack(void);
static void gargoyleLanceStabInit(void);
static void gargoyleLanceStabMoveToTarget(void);
static void gargoyleLanceStab(void);
static void gargoyleLanceStabReactToBlock(Entity *);
static void gargoyleLanceStabFinish(void);
static void gargoyleInvisibleAttackInit(void);
static void gargoyleBecomeInvisible(void);
static void gargoyleInvisibleAttackMoveToTop(void);
static void gargoyleInvisibleAttackFollowPlayer(void);
static void gargoyleInvisibleDrop(void);
static void gargoyleInvisibleDropWait(void);
static void gargoyleLanceThrowInit(void);
static void gargoyleLanceThrowMoveToTarget(void);
static void gargoyleLanceThrow(void);
static void gargoyleLanceThrowWait(void);
static void gargoyleCreateLanceInit(void);
static void gargoyleCreateLance(void);
static void gargoyleCreateLanceWait(void);
static void gargoyleLanceAppearWait(void);
static void gargoyleLanceAppearFinish(void);
static void gargoyleLanceWait(void);
static void gargoyleLanceDrop(void);
static void gargoyleLanceDie(void);
static void gargoylePetrifyAttackInit(void);
static void gargoylePetrifyAttack(void);
static void gargoylePetrifyAttackWait(void);
static void gargoyleWeaponRemoveBlastInit(void);
static void gargoyleWeaponRemoveBlast(void);
static void gargoyleBlastRemoveWeapon(Entity *);
static void gargoyleWeaponRemoveBlastFinish(void);
static void gargoyleTakeDamage(Entity *, int);
static void gargoyleLanceAttack3(void);
static void gargoyleFakeLanceDropInit(void);
static void gargoyleFakeLanceDropAppear(void);
static void gargoyleFakeLanceDropWait(void);
static void gargoyleFakeLanceDrop(void);
static void gargoyleFakeLanceDropExplodeWait(void);
static void gargoyleLanceExplode(void);
static void gargoyleFakeLanceDie(void);
static void gargoyleDie(void);
static void gargoyleCloneCheck(void);
static void gargoyleCloneDie(void);
static void gargoyleAddStoneCoat(void);
static void gargoyleCoatWait(void);

static void becomeGuardian(void);
static void guardianBodyWait(void);
static void guardianInitialise(void);
static void guardianHeadWait(void);
static void guardianRiseUp(void);
static void guardianCreateBody(void);
static void guardianSetSnakePosition(int, int);
static void guardianAlignBodyToHead(void);
static void guardianBiteAttackInit(void);
static void guardianBiteAttackWindUp(void);
static void guardianBiteAttack(void);
static void guardianReturnToStart(void);
static void guardianAttackFinished(void);
static void guardianDie(void);
static void guardianDieWait(void);
static void guardianFallToGround(void);
static void guardianChangeSidesInit(void);
static void guardianChangeSides(void);
static void guardianRiseUpWait(void);
static void guardianTakeDamage(Entity *, int);
static void guardianShotAttackInit(void);
static void guardianShotAttackWindUp(void);
static void guardianShotAttack(void);
static void guardianSpecialShotWait(void);
static void guardianSpecialShotBlock(Entity *);
static void guardianSpecialShotTouch(Entity *);
static void guardianBiteReactToBlock(Entity *);
static void guardianCrushAttackInit(void);
static void guardianCrushAttackMoveToPosition(void);
static void guardianCrushAttack(void);
static void guardianCrushAttackFinish(void);
static void guardianAddSmokeAlongBody(void);
static void guardianStunned(void);
static void guardianCrushAttackHit(Entity *);
static void guardianBodyTakeDamage(Entity *, int);
static void guardianStarWait(void);

Entity *addBlackBook2(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Black Book");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->die = NULL;
	e->takeDamage = NULL;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

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

			self->action = &doIntro;

			self->thinkTime = 30;
		}
	}
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setContinuePoint(FALSE, self->name, NULL);

		addBack();

		self->targetX = self->startX;
		self->targetY = self->startY;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		self->thinkTime = 120;

		self->action = &introPause;
	}
}

static void addBack()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Black Book Back");
	}

	loadProperties("boss/black_book_back", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &backWait;

	e->creditsAction = &backWait;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->head = self;

	setEntityAnimation(e, "STAND");
}

static void introPause()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->targetX = 0;

		initBossHealthBar();

		playDefaultBossMusic();

		self->action = &blackBookAttackFinished;
	}

	hover();
}

static void blackBookWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		switch (self->targetX)
		{
			case 0:
				self->action = &becomeKingGrub;
			break;

			case 1:
				self->action = &becomeQueenWasp;
			break;

			case 2:
				self->action = &becomeGuardian;
			break;

			case 3:
				self->action = &becomeGargoyle;
			break;

			case 4:
				self->action = &becomeBlob;
			break;

			default:
				self->action = &becomeAwesome;
			break;
		}

		self->targetX++;
	}

	hover();
}

static void becomeGargoyle()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Black Book Gargoyle");
	}

	loadProperties("boss/gargoyle", e);

	e->maxHealth = e->health = 500;

	e->flags |= LIMIT_TO_SCREEN;

	e->face = self->face;

	setEntityAnimation(e, "FACE_FRONT_CROUCH");

	e->x = self->x;
	e->y = self->y;

	e->targetX = e->x;
	e->targetY = getMapFloor(e->x, e->y) - e->h;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	e->flags &= ~FLY;

	e->action = &gargoyleInitialise;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &entityTouch;

	e->takeDamage = &gargoyleTakeDamage;

	e->die = &gargoyleDie;

	e->head = self;

	self->flags |= NO_DRAW;

	self->mental = 1;

	self->action = &transformWait;
}

static void gargoyleInitialise()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime = 2;

		self->takeDamage = &gargoyleTakeDamage;

		self->action = &gargoyleWait;

		self->thinkTime = 90;

		initEnergyBar(self);
	}
}

static void gargoyleAttackFinished()
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

	self->action = &gargoyleAttackFinishedMoveUp;

	checkToMap(self);
}

static void gargoyleAttackFinishedMoveUp()
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

			self->action = &gargoyleAttackFinishedMoveHorizontal;
		}
	}

	checkToMap(self);
}

static void gargoyleAttackFinishedMoveHorizontal()
{
	if (atTarget())
	{
		self->thinkTime = 0;

		self->flags &= ~UNBLOCKABLE;

		self->action = &gargoyleWait;
	}

	facePlayer();

	checkToMap(self);
}

static void gargoyleWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		self->startX = getMapStartX();
		self->endX   = getMapStartX() + SCREEN_WIDTH - self->w - 1;

		if ((self->target == NULL || self->target->inUse == FALSE) && self->maxThinkTime < 3)
		{
			self->action = &gargoyleCreateLanceInit;
		}

		else
		{
			switch (self->maxThinkTime)
			{
				case 2:
					if (self->health == self->maxHealth / 4)
					{
						self->action = &gargoyleLanceThrowInit;
					}

					else
					{
						if (player.element == SLIME)
						{
							self->action = &gargoylePetrifyAttackInit;
						}

						else
						{
							switch (prand() % 4)
							{
								case 0:
									self->action = &gargoyleLanceStabInit;
								break;

								case 1:
									self->action = &gargoyleWeaponRemoveBlastInit;
								break;

								case 2:
									self->action = &gargoylePetrifyAttackInit;
								break;

								default:
									self->action = &gargoyleInvisibleAttackInit;
								break;
							}
						}
					}
				break;

				case 3:
					self->action = &gargoyleSplitInHalfInit;
				break;

				default:
					switch (prand() % 2)
					{
						case 0:
							self->action = &gargoyleDropAttackInit;
						break;

						case 1:
							self->action = &gargoyleWeaponRemoveBlastInit;
						break;
					}
				break;
			}
		}
	}

	checkToMap(self);
}

static void gargoyleSplitInHalfInit()
{
	self->maxThinkTime = 4;

	setEntityAnimation(self, "DROP_ATTACK");

	self->thinkTime = 60;

	self->action = &gargoyleSplitInHalf;

	checkToMap(self);
}

static void gargoyleSplitInHalf()
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

		e->action = &gargoyleSplitInHalfMove;

		e->draw = &drawLoopingAnimationToMap;

		e->touch = &entityTouch;

		e->takeDamage = &entityTakeDamageNoFlinch;

		e->die = &gargoyleCloneDie;

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

		self->action = &gargoyleSplitInHalfMove;

		self->targetX = self->x + self->w;

		self->targetY = self->y;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->target = e;

		self->thinkTime = 0;
	}
}

static void gargoyleSplitInHalfMove()
{
	checkToMap(self);

	if (atTarget() || self->dirX == 0)
	{
		if (self->head != NULL && self->head->dirX == 0)
		{
			self->flags &= ~NO_DRAW;

			self->action = &gargoyleAttackFinished;
		}

		else if (self->target != NULL && self->target->dirX == 0)
		{
			self->flags &= ~NO_DRAW;

			self->action = &gargoyleAttackFinished;
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

static void gargoyleDropAttackInit()
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

	self->action = &gargoyleDropAttackMoveToTop;

	checkToMap(self);

	gargoyleCloneCheck();
}

static void gargoyleDropAttackMoveToTop()
{
	if (atTarget())
	{
		setEntityAnimation(self, "DROP_ATTACK_READY");

		self->action = &gargoyleDropAttackFollowPlayer;
	}

	checkToMap(self);

	gargoyleCloneCheck();
}

static void gargoyleDropAttackFollowPlayer()
{
	float target;

	target = player.x - self->w / 2 + player.w / 2;

	/* Move above the player */

	if (fabs(target - self->x) <= fabs(self->dirX))
	{
		self->targetY = self->y - self->h;

		self->dirX = 0;

		self->thinkTime = 30;

		self->action = &gargoyleDropAttack;
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

				self->action = &gargoyleDropAttack;
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

				self->action = &gargoyleDropAttack;
			}
		}
	}

	gargoyleCloneCheck();
}

static void gargoyleDropAttack()
{
	int i;
	long onGround = self->flags & ON_GROUND;
	Entity *e;

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "DROP_ATTACK");

		self->flags &= ~FLY;

		if (landedOnGround(onGround) == TRUE)
		{
			playSoundToMap("sound/enemy/red_grub/thud", -1, self->x, self->y, 0);

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
				self->action = &gargoyleAttackFinished;
			}
		}
	}

	gargoyleCloneCheck();
}

static void gargoyleLanceStabInit()
{
	self->thinkTime = 30;

	self->action = &gargoyleLanceStabMoveToTarget;

	checkToMap(self);
}

static void gargoyleLanceStabMoveToTarget()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		facePlayer();

		calculatePath(self->x, self->y, player.x + player.w / 2 - self->w / 2, player.y + player.h / 2 - self->h / 2, &self->dirX, &self->dirY);

		self->dirX *= 16;
		self->dirY *= 16;

		self->action = &gargoyleLanceStab;

		self->reactToBlock = &gargoyleLanceStabReactToBlock;

		playSoundToMap("sound/boss/gargoyle/gargoyle_lance_stab", -1, self->x, self->y, 0);
	}

	checkToMap(self);
}

static void gargoyleLanceStab()
{
	if (self->dirX == 0 || self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		self->flags &= ~FLY;

		self->dirX = self->face == RIGHT ? -5 : 5;
		self->dirY = -6;

		self->action = &gargoyleLanceStabFinish;

		self->thinkTime = 30;
	}

	checkToMap(self);
}

static void gargoyleLanceStabReactToBlock(Entity *other)
{
	self->dirX = 0;
}

static void gargoyleLanceStabFinish()
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
			self->action = &gargoyleAttackFinished;
		}
	}
}

static void gargoyleInvisibleAttackInit()
{
	self->flags &= ~FLY;

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		self->action = &gargoyleBecomeInvisible;

		self->endY = self->y + self->h;
	}

	checkToMap(self);
}

static void gargoyleBecomeInvisible()
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

		self->action = &gargoyleInvisibleAttackMoveToTop;

		checkToMap(self);
	}
}

static void gargoyleInvisibleAttackMoveToTop()
{
	if (atTarget())
	{
		setEntityAnimation(self, "DROP_ATTACK");

		self->action = &gargoyleInvisibleAttackFollowPlayer;
	}

	checkToMap(self);
}

static void gargoyleInvisibleAttackFollowPlayer()
{
	float target;

	target = player.x - self->w / 2 + player.w / 2;

	/* Move above the player */

	if (fabs(target - self->x) <= fabs(self->dirX))
	{
		self->targetY = self->y - self->h;

		self->thinkTime = 60;

		self->dirX = 0;

		self->action = &gargoyleInvisibleDrop;
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

				self->action = &gargoyleInvisibleDrop;
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

				self->action = &gargoyleInvisibleDrop;
			}
		}
	}
}

static void gargoyleInvisibleDrop()
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

			playSoundToMap("sound/enemy/red_grub/thud", -1, self->x, self->y, 0);

			self->flags &= ~NO_DRAW;

			self->mental--;

			self->thinkTime = self->mental <= 0 ? 30 : 60;

			self->action = &gargoyleInvisibleDropWait;
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

static void gargoyleInvisibleDropWait()
{
	Target *t;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental <= 0)
		{
			self->action = &gargoyleAttackFinished;
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

			self->action = &gargoyleInvisibleAttackMoveToTop;
		}
	}

	checkToMap(self);
}

static void gargoyleLanceThrowInit()
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

	self->action = &gargoyleLanceThrowMoveToTarget;

	self->thinkTime = 30;

	checkToMap(self);
}

static void gargoyleLanceThrowMoveToTarget()
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

			self->action = &gargoyleLanceThrow;
		}
	}

	checkToMap(self);
}

static void gargoyleLanceThrow()
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

			playSoundToMap("sound/boss/gargoyle/gargoyle_lance_stab", -1, self->x, self->y, 0);

			self->target->mental = -1;

			self->target->dirY = 4;

			self->action = &gargoyleLanceThrowWait;

			self->thinkTime = 30;

			self->maxThinkTime++;
		}
	}

	checkToMap(self);
}

static void gargoyleLanceThrowWait()
{
	if (self->target->inUse == FALSE)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &gargoyleAttackFinished;
		}
	}

	checkToMap(self);
}

static void gargoyleCreateLanceInit()
{
	self->flags &= ~FLY;

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		self->thinkTime = 30;

		self->action = &gargoyleCreateLance;
	}

	checkToMap(self);
}

static void gargoyleCreateLance()
{
	Entity *e;

	setEntityAnimation(self, "CREATE_LANCE");

	self->face = RIGHT;

	self->mental = 1;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		playSoundToMap("sound/boss/gargoyle/gargoyle_create_lance", -1, self->x, self->y, 0);

		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Gargoyle's Lance");
		}

		loadProperties("boss/gargoyle_lance_3", e);

		e->action = &gargoyleLanceAppearWait;

		e->draw = &drawLoopingAnimationToMap;

		e->type = ENEMY;

		e->head = self;

		setEntityAnimation(e, "LANCE_APPEAR");

		e->animationCallback = &gargoyleLanceAppearFinish;

		self->target = e;

		e->maxThinkTime = self->maxThinkTime;

		self->action = &gargoyleCreateLanceWait;

		self->thinkTime = 90;
	}

	checkToMap(self);
}

static void gargoyleCreateLanceWait()
{
	if (self->mental == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->health = self->maxHealth;

			facePlayer();

			setEntityAnimation(self, "STAND");

			self->action = &gargoyleAttackFinished;
		}
	}

	checkToMap(self);
}

static void gargoyleLanceAppearWait()
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

static void gargoyleLanceAppearFinish()
{
	self->head->mental = 0;

	self->action = &gargoyleLanceWait;
}

static void gargoyleLanceWait()
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

		self->action = &gargoyleLanceDrop;
	}

	self->alpha = self->head->alpha;
}

static void gargoyleLanceDrop()
{
	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		self->takeDamage = &entityTakeDamageNoFlinch;

		self->die = &gargoyleLanceDie;

		setEntityAnimation(self, "LANCE_IN_GROUND");

		playSoundToMap("sound/enemy/ground_spear/spear", -1, self->x, self->y, 0);

		self->action = &gargoyleLanceAttack3;

		self->thinkTime = 30;
	}

	checkToMap(self);
}

static void gargoyleLanceDie()
{
	int i;
	Entity *e;
	char name[MAX_VALUE_LENGTH];

	playSoundToMap("sound/enemy/centurion/centurion_die", -1, self->x, self->y, 0);

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

static void gargoylePetrifyAttackInit()
{
	self->flags &= ~FLY;

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime = 30;

		self->action = &gargoylePetrifyAttack;

		self->mental = 0;
	}

	checkToMap(self);
}

static void gargoylePetrifyAttack()
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
			playSoundToMap("sound/boss/gargoyle/gargoyle_petrify", -1, self->x, self->y, 0);

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

			self->action = &gargoylePetrifyAttackWait;
		}
	}

	checkToMap(self);
}

static void gargoylePetrifyAttackWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &gargoyleAttackFinished;
	}

	checkToMap(self);
}

static void gargoyleWeaponRemoveBlastInit()
{
	self->flags &= ~FLY;

	if (self->standingOn != NULL || (self->flags & ON_GROUND))
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime = 30;

		self->action = &gargoyleWeaponRemoveBlast;

		self->mental = 1 + prand() % 3;
	}

	checkToMap(self);

	gargoyleCloneCheck();
}

static void gargoyleWeaponRemoveBlast()
{
	Entity *e;

	facePlayer();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "WEAPON_REMOVE");

		playSoundToMap("sound/boss/snake_boss/snake_boss_shot", -1, self->x, self->y, 0);

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

		e->touch = &gargoyleBlastRemoveWeapon;

		e->flags |= FLY;

		e->thinkTime = 1200;

		self->action = &gargoyleWeaponRemoveBlastFinish;

		self->mental--;

		self->thinkTime = self->mental > 0 ? 30 : 60;
	}

	checkToMap(self);

	gargoyleCloneCheck();
}

static void gargoyleBlastRemoveWeapon(Entity *other)
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

		playSoundToMap("sound/common/punch", EDGAR_CHANNEL, self->x, self->y, 0);

		setCustomAction(other, &invulnerable, 60, 0, 0);

		setPlayerStunned(30);

		other->x -= other->dirX;
		other->y -= other->dirY;

		other->dirX = (6 + prand() % 3) * (self->dirX < 0 ? -1 : 1);
		other->dirY = -8;

		self->inUse = FALSE;
	}
}

static void gargoyleWeaponRemoveBlastFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = self->mental > 0 ? &gargoyleWeaponRemoveBlast : &gargoyleAttackFinished;
	}

	checkToMap(self);

	gargoyleCloneCheck();
}

static void gargoyleTakeDamage(Entity *other, int damage)
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

			enemyPain();

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
				self->mental = 0;

				self->thinkTime = 180;

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

static void gargoyleLanceAttack3()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime = 0;

		self->action = &gargoyleFakeLanceDropInit;
	}

	checkToMap(self);
}

static void gargoyleFakeLanceDropInit()
{
	Target *t = getTargetByName("GARGOYLE_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Lance cannot find target");
	}

	self->flags |= (FLY|NO_DRAW);

	setEntityAnimation(self, "LANCE_THROW");

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

	self->y = t->y;

	self->thinkTime = 30;

	self->action = &gargoyleFakeLanceDropAppear;
}

static void gargoyleFakeLanceDropAppear()
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
				e->die = &gargoyleFakeLanceDie;
				e->pain = &enemyPain;

				e->type = ENEMY;

				setEntityAnimation(e, "LANCE_THROW");

				e->mental = 0;

				e->head = self;
			}

			e->action = &gargoyleFakeLanceDropWait;

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

static void gargoyleFakeLanceDropWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~NO_DRAW;

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

		self->action = &gargoyleFakeLanceDrop;

		self->thinkTime = self->maxThinkTime;
	}

	checkToMap(self);
}

static void gargoyleFakeLanceDrop()
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

			playSoundToMap("sound/enemy/ground_spear/spear", -1, self->x, self->y, 0);
		}

		else
		{
			self->thinkTime = 180 + prand() % 120;
		}

		self->action = &gargoyleFakeLanceDropExplodeWait;
	}

	checkToMap(self);
}

static void gargoyleFakeLanceDropExplodeWait()
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
			self->action = &gargoyleLanceExplode;
		}
	}

	else if (self->endX <= 0)
	{
		self->action = &gargoyleFakeLanceDropInit;
	}

	checkToMap(self);
}

static void gargoyleLanceExplode()
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

	playSoundToMap("sound/common/explosion", -1, self->x, self->y, 0);

	self->head->endX--;

	self->inUse = FALSE;
}

static void gargoyleFakeLanceDie()
{
	self->head->endX--;

	entityDieNoDrop();
}

static void gargoyleDie()
{
	int i;
	long onGround;
	Entity *e;

	self->action = &gargoyleDie;

	self->damage = 0;

	self->takeDamage = NULL;

	setEntityAnimation(self, "FACE_FRONT_CROUCH");

	self->dirX = 0;

	self->flags &= ~(FLY|LIMIT_TO_SCREEN);

	onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (landedOnGround(onGround) == TRUE)
	{
		playSoundToMap("sound/enemy/red_grub/thud", -1, self->x, self->y, 0);

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
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->mental == 0)
			{
				gargoyleAddStoneCoat();

				self->target->alpha = 0;

				self->mental = 1;
			}

			else if (self->mental == 2)
			{
				self->targetX = self->head->x + self->head->w / 2 - self->w / 2;
				self->targetY = self->head->y + self->head->h / 2 - self->h / 2;

				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

				self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

				playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

				self->action = &transformRemove;
			}
		}
	}
}

static void gargoyleAddStoneCoat()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Gargoyle Stone Coat");
	}

	loadProperties("boss/gargoyle_stone_coat", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &gargoyleCoatWait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->takeDamage = NULL;

	e->type = ENEMY;

	e->head = self;

	self->target = e;

	e->thinkTime = 30;

	setEntityAnimation(e, getAnimationTypeAtIndex(self));
}

static void gargoyleCoatWait()
{
	self->face = self->head->face;

	setEntityAnimation(self, getAnimationTypeAtIndex(self->head));

	self->alpha++;

	if (self->alpha >= 255)
	{
		self->thinkTime--;

		self->alpha = 255;

		if (self->thinkTime <= 0)
		{
			self->head->mental = 2;
		}
	}

	self->x = self->head->x;
	self->y = self->head->y;

	if (self->head->flags & NO_DRAW)
	{
		self->inUse = FALSE;
	}
}

static void gargoyleCloneCheck()
{
	if (self->head != NULL && self->head->health <= 0)
	{
		self->action = &gargoyleCloneDie;
	}
}

static void gargoyleCloneDie()
{
	self->action = &gargoyleCloneDie;

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

static void becomeGuardian()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Black Book Guardian");
	}

	loadProperties("boss/snake_boss", e);

	e->maxHealth = e->health = 500;

	e->flags |= LIMIT_TO_SCREEN;

	e->face = self->face;

	setEntityAnimation(e, "STAND");

	e->x = self->x;
	e->y = self->y;

	e->targetX = e->x;
	e->targetY = getMapFloor(e->x, e->y) + e->h;

	e->layer = BACKGROUND_LAYER;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	e->action = &guardianInitialise;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &entityTouch;

	e->takeDamage = &guardianTakeDamage;

	e->head = self;

	self->flags |= NO_DRAW;

	self->mental = 1;

	self->action = &transformWait;
}

static void guardianInitialise()
{
	Target *t;

	self->flags |= NO_DRAW;

	guardianCreateBody();

	t = getTargetByName("SNAKE_BOSS_TARGET_LEFT");

	if (t == NULL)
	{
		showErrorAndExit("Snake boss cannot find target");
	}

	guardianSetSnakePosition(t->x, t->y);

	self->targetY = self->y - 64;

	self->thinkTime = 60;

	self->face = RIGHT;

	self->flags |= LIMIT_TO_SCREEN;

	self->layer = MID_GROUND_LAYER;

	self->action = &guardianRiseUp;

	initEnergyBar(self);
}

static void guardianBodyWait()
{
	if (self->head->flags & FLASH)
	{
		self->flags |= FLASH;
	}

	else
	{
		self->flags &= ~FLASH;
	}

	checkToMap(self);
}

static void guardianHeadWait()
{
	int x;

	/* Sway back and forth */

	self->dirX += 0.5;

	if (self->dirX >= 360)
	{
		self->dirX = 0;
	}

	x = 24;

	self->x = self->targetX + (sin(DEG_TO_RAD(self->dirX)) * x);

	guardianAlignBodyToHead();

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		self->thinkTime = 0;

		x = prand() % 4;

		switch (x)
		{
			case 0:
				self->action = &guardianBiteAttackInit;
			break;

			case 1:
				self->action = &guardianChangeSidesInit;
			break;

			case 2:
				self->action = &guardianShotAttackInit;
			break;

			default:
				self->action = &guardianCrushAttackInit;
			break;
		}
	}

	if (prand() % 180 == 0)
	{
		playSoundToMap("sound/boss/snake_boss/hiss", BOSS_CHANNEL, self->x, self->y, 0);
	}
}

static void guardianRiseUp()
{
	Entity *e, *smoke;

	/*facePlayer();*/

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->damage = 1;

		self->y -= 2;

		self->thinkTime = 0;

		self->flags &= ~NO_DRAW;

		self->takeDamage = &guardianTakeDamage;
	}

	guardianAlignBodyToHead();

	e = self->target;

	while (e != NULL)
	{
		if (e->target == NULL)
		{
			smoke = addSmoke(e->x + (prand() % self->w) * (prand() % 2 == 0 ? -1 : 1), e->y + prand() % e->h, "decoration/dust");

			if (smoke != NULL)
			{
				smoke->dirY = 0;
			}
		}

		e = e->target;
	}

	if (self->y <= self->targetY)
	{
		self->y = self->targetY;

		self->targetX = self->x;

		self->dirX = 0;

		self->thinkTime = 120;

		self->action = &guardianHeadWait;
	}
}

static void guardianCreateBody()
{
	char bodyName[MAX_VALUE_LENGTH];
	int i;
	Entity **body, *head;

	body = malloc(self->mental * sizeof(Entity *));

	if (body == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Snake Boss body...", self->mental * (int)sizeof(Entity *));
	}

	snprintf(bodyName, sizeof(bodyName), "%s_body", self->name);

	for (i=self->mental-1;i>=0;i--)
	{
		body[i] = getFreeEntity();

		if (body[i] == NULL)
		{
			showErrorAndExit("No free slots to add a Snake Boss body part");
		}

		loadProperties(bodyName, body[i]);

		body[i]->x = self->x;
		body[i]->y = self->y;

		body[i]->action = &guardianBodyWait;

		body[i]->draw = &drawLoopingAnimationToMap;
		body[i]->touch = &entityTouch;
		body[i]->die = &entityDieNoDrop;
		body[i]->takeDamage = &guardianBodyTakeDamage;

		body[i]->type = ENEMY;

		body[i]->active = FALSE;

		body[i]->flags |= NO_DRAW;

		setEntityAnimation(body[i], "STAND");
	}

	/* Recreate the head so that it's on top */

	head = getFreeEntity();

	if (head == NULL)
	{
		showErrorAndExit("No free slots to add a Snake Boss head");
	}

	*head = *self;

	self->inUse = FALSE;

	self = head;

	/* Link the sections */

	for (i=self->mental-1;i>=0;i--)
	{
		if (i == 0)
		{
			self->target = body[i];
		}

		else
		{
			body[i - 1]->target = body[i];
		}

		body[i]->head = self;
	}

	free(body);
}

static void guardianBiteAttackInit()
{
	/*facePlayer();*/

	setEntityAnimation(self, "ATTACK_1");

	self->targetX = self->endX + (self->face == LEFT ? -32 : 32);
	self->targetY = self->endY - 32;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= 4;
	self->dirY *= 4;

	self->action = &guardianBiteAttackWindUp;

	self->maxThinkTime = 1 + prand() % 3;
}

static void guardianBiteAttackWindUp()
{
	checkToMap(self);

	if (atTarget())
	{
		self->targetX = self->face == RIGHT ? self->x + 320 - self->w - 1 : self->x - 320;

		self->action = &guardianBiteAttack;

		self->reactToBlock = &guardianBiteReactToBlock;

		self->dirX = (self->targetX < self->x ? -self->speed * 2 : self->speed * 2);
	}

	guardianAlignBodyToHead();
}

static void guardianBiteAttack()
{
	checkToMap(self);

	if (fabs(self->targetX - self->x) <= fabs(self->dirX))
	{
		self->x = self->targetX;

		self->maxThinkTime--;

		if (self->maxThinkTime <= 0)
		{
			self->action = &guardianAttackFinished;
		}

		else
		{
			self->targetX = self->endX + (self->face == LEFT ? -32 : 32);
			self->targetY = self->endY - 32;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= 4;
			self->dirY *= 4;

			self->action = &guardianBiteAttackWindUp;
		}
	}

	guardianAlignBodyToHead();
}

static void guardianShotAttackInit()
{
	/*facePlayer();*/

	setEntityAnimation(self, "ATTACK_1");

	self->targetX = self->endX + (self->face == LEFT ? -50 : 50);
	self->targetY = self->endY - 32;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= 4;
	self->dirY *= 4;

	self->action = &guardianShotAttackWindUp;

	self->flags |= UNBLOCKABLE;

	self->startX = 0;
}

static void guardianShotAttackWindUp()
{
	checkToMap(self);

	if (atTarget())
	{
		self->maxThinkTime = 5;

		self->thinkTime = 0;

		self->action = &guardianShotAttack;
	}

	guardianAlignBodyToHead();
}

static void guardianShotAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		playSoundToMap("sound/boss/snake_boss/snake_boss_shot", BOSS_CHANNEL, self->x, self->y, 0);

		if (prand() % 4 == 0 && self->startX == 0)
		{
			e = addProjectile("boss/snake_boss_special_shot", self, self->x + self->w / 2, self->y + self->h / 2, (self->face == RIGHT ? 7 : -7), 0);

			e->reactToBlock = &guardianSpecialShotBlock;

			self->startX = 1;
		}

		else
		{
			e = addProjectile("boss/snake_boss_normal_shot", self, self->x + self->w / 2, self->y + self->h / 2, (self->face == RIGHT ? 7 : -7), 0);

			e->reactToBlock = &bounceOffShield;
		}

		e->y -= e->h / 2;

		self->x += (self->face == LEFT ? 10 : -10);

		self->maxThinkTime--;

		if (self->maxThinkTime <= 0)
		{
			self->action = &guardianAttackFinished;
		}

		else
		{
			self->thinkTime = 10;
		}
	}

	guardianAlignBodyToHead();
}

static void guardianSetSnakePosition(int x, int y)
{
	Entity *e;

	self->x = x;
	self->y = y;

	self->endX = x;
	self->endY = y;

	e = self->target;

	while (e != NULL)
	{
		e->y = y;
		e->x = x + 23;

		e = e->target;
	}
}

static void guardianChangeSidesInit()
{
	self->targetX = self->endX;
	self->targetY = self->endY;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= 2;
	self->dirY *= 2;

	self->action = &guardianChangeSides;
}

static void guardianChangeSides()
{
	int side;
	Entity *e, *smoke;
	Target *t;

	self->x += self->dirX;
	self->y += self->dirY;

	if (atTarget())
	{
		self->flags |= NO_DRAW;

		self->takeDamage = NULL;

		self->targetX = self->x;

		self->dirX = 0;

		self->thinkTime = 120;

		side = prand() % 2;

		if (side == 0)
		{
			t = getTargetByName("SNAKE_BOSS_TARGET_LEFT");

			self->face = RIGHT;
		}

		else
		{
			t = getTargetByName("SNAKE_BOSS_TARGET_RIGHT");

			self->face = LEFT;
		}

		if (t == NULL)
		{
			showErrorAndExit("Snake boss cannot find target");
		}

		self->damage = 0;

		guardianSetSnakePosition(t->x, t->y);

		self->targetX = self->x;
		self->targetY = self->y - 64;

		self->thinkTime = 120;

		self->action = &guardianRiseUpWait;
	}

	else
	{
		e = self->target;

		while (e != NULL)
		{
			if (e->target == NULL)
			{
				smoke = addSmoke(e->x + (prand() % self->w) * (prand() % 2 == 0 ? -1 : 1), e->y + prand() % e->h, "decoration/dust");

				if (smoke != NULL)
				{
					smoke->dirY = 0;
				}
			}

			e = e->target;
		}
	}

	guardianAlignBodyToHead();
}

static void guardianCrushAttackInit()
{
	self->maxThinkTime = 5;

	self->targetX = self->endX + (self->face == LEFT ? -75 : 75);
	self->targetY = self->endY - 128;

	self->action = &guardianCrushAttackMoveToPosition;
}

static void guardianCrushAttackMoveToPosition()
{
	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= 9;
	self->dirY *= 9;

	checkToMap(self);

	if (atTarget())
	{
		self->y = self->targetY;

		self->x = self->targetX;

		self->dirY = 0;

		self->dirX = 0;

		self->action = &guardianCrushAttack;
	}

	guardianAlignBodyToHead();
}

static void guardianCrushAttack()
{
	self->dirX = 0;

	self->dirY = 8;

	self->flags |= UNBLOCKABLE;

	self->flags &= ~FLY;

	self->action = &guardianCrushAttackFinish;

	self->touch = &guardianCrushAttackHit;

	self->thinkTime = self->maxThinkTime == 1 ? 60 : 15;
}

static void guardianCrushAttackFinish()
{
	long onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (onGround == 0)
		{
			guardianAddSmokeAlongBody();

			self->touch = &entityTouch;
		}

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->maxThinkTime--;

			if (self->maxThinkTime <= 0)
			{
				self->action = &guardianAttackFinished;
			}

			else
			{
				self->targetX = self->targetX + (self->face == LEFT ? -75 : 75);
				self->targetY = self->endY - 128;

				self->action = &guardianCrushAttackMoveToPosition;
			}
		}
	}

	guardianAlignBodyToHead();
}

static void guardianCrushAttackHit(Entity *other)
{
	if (other->type == PLAYER)
	{
		self->thinkTime = 0;

		self->maxThinkTime = 0;
	}
}

static void guardianRiseUpWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 120;

		self->action = &guardianRiseUp;
	}
}

static void guardianAttackFinished()
{
	setEntityAnimation(self, "STAND");

	self->flags &= ~UNBLOCKABLE;

	self->flags |= FLY;

	self->targetX = self->endX;
	self->targetY = self->endY - 64;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= self->speed;
	self->dirY *= self->speed;

	self->action = &guardianReturnToStart;
}

static void guardianReturnToStart()
{
	checkToMap(self);

	if (atTarget())
	{
		self->y = self->targetY;

		self->targetX = self->x;

		self->dirX = 0;

		self->thinkTime = 120;

		self->action = &guardianHeadWait;
	}

	guardianAlignBodyToHead();
}

static void guardianAlignBodyToHead()
{
	float x, y, partDistanceX, partDistanceY;
	Entity *e;

	x = self->x;
	y = self->y;

	partDistanceX = self->endX - self->x;
	partDistanceY = fabs(self->endY - self->y);

	partDistanceX /= self->mental;
	partDistanceY /= self->mental;

	e = self->target;

	while (e != NULL)
	{
		x += partDistanceX;
		y += partDistanceY;

		e->x = (e->target == NULL ? self->endX : x) + 23;
		e->y = (e->target == NULL ? self->endY : y);

		e->damage = self->damage;

		e->face = self->face;

		if (self->flags & NO_DRAW)
		{
			e->flags |= NO_DRAW;
		}

		else
		{
			e->flags &= ~NO_DRAW;
		}

		e = e->target;
	}
}

static void guardianTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		/* Only takes proper damage against its own shot */

		if (strcmpignorecase(other->name, "boss/snake_boss_special_shot") == 0 && other->parent->type == PLAYER)
		{
			self->health -= damage;

			self->flags &= ~FLY;

			self->dirY = -2;

			self->dirX = self->face == LEFT ? -4 : 4;

			self->action = &guardianStunned;
		}

		else
		{
			damage = (self->flags & HELPLESS ? damage * 3 : damage);

			self->health -= damage;
		}

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);
			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

			enemyPain();
		}

		else
		{
			setEntityAnimation(self, "PAIN");

			self->health = 0;

			self->thinkTime = 60;

			self->damage = 0;

			self->takeDamage = NULL;
			self->touch = NULL;

			self->targetX = self->endX;
			self->targetY = self->endY - 128;

			clearCustomActions(self);

			self->action = &guardianDie;

			self->dirX = self->face == RIGHT ? 2 : -2;

			self->flags &= ~(HELPLESS|FLY);
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

static void guardianDie()
{
	long onGround = self->flags & ON_GROUND;

	self->action = &guardianDie;

	checkToMap(self);

	if (self->flags & FLY)
	{
		if (atTarget())
		{
			self->dirX = self->dirY = 0;

			self->thinkTime = 120;

			self->action = &guardianFallToGround;
		}
	}

	else
	{
		if (self->flags & ON_GROUND)
		{
			self->dirX = 0;

			if (onGround == 0)
			{
				guardianAddSmokeAlongBody();
			}

			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

				self->dirX *= 2;
				self->dirY *= 2;

				self->flags |= FLY;
			}
		}
	}

	guardianAlignBodyToHead();
}

static void guardianFallToGround()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		playSoundToMap("sound/boss/snake_boss/snake_boss_die", BOSS_CHANNEL, self->x, self->y, 0);

		self->flags &= ~FLY;

		self->dirX = self->face == LEFT ? -8 : 8;

		setEntityAnimation(self, "DIE");
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		playSoundToMap("sound/common/crash", BOSS_CHANNEL, self->x, self->y, 0);

		setEntityAnimation(self, "PAIN");

		self->thinkTime = 180;

		self->dirX = 0;

		self->action = &guardianDieWait;

		addSmokeAlongBody();

		shakeScreen(MEDIUM, 90);

		e = self->target;

		while (e != NULL)
		{
			e->flags &= ~FLY;

			e = e->target;
		}
	}

	else
	{
		guardianAlignBodyToHead();
	}
}

static void guardianDieWait()
{
	Entity *e;

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = self;

		self = self->target;

		while (self != NULL)
		{
			self->die();

			self = self->target;
		}

		self = e;

		self->targetX = self->head->x + self->head->w / 2 - self->w / 2;
		self->targetY = self->head->y + self->head->h / 2 - self->h / 2;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &transformRemove;
	}
}

static void guardianSpecialShotBlock(Entity *other)
{
	self->dirX = (self->dirX < 0 ? 5 : -5);

	self->dirY = -5;

	self->type = ENEMY;

	self->target = self->parent;

	self->parent = NULL;

	self->flags &= ~FLY;

	self->action = &guardianSpecialShotWait;

	self->touch = NULL;

	self->thinkTime = 300;

	self->die = &entityDieNoDrop;
}

static void guardianSpecialShotWait()
{
	self->thinkTime--;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->touch = &guardianSpecialShotTouch;

		self->dirX = 0;
	}

	if (self->thinkTime <= 0)
	{
		self->die();
	}
}

static void guardianSpecialShotTouch(Entity *other)
{
	Entity *e;

	if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		e = addProjectile("boss/snake_boss_special_shot", &player, self->x, self->y, 0, 0);

		e->targetX = self->target->x + self->target->w / 2;
		e->targetY = self->target->y + self->target->h / 2;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->dirX *= 12;
		e->dirY *= 12;

		e->dirX = (player.face == RIGHT ? fabs(e->dirX) : fabs(e->dirX) * -1);

		e->damage = 10;

		self->inUse = FALSE;
	}
}

static void guardianBiteReactToBlock(Entity *other)
{
	self->targetX = self->x;

	self->x = (int)self->x;

	self->dirX = 0;
}

static void guardianStunned()
{
	int i;
	long onGround = self->flags & ON_GROUND;
	Entity *e;

	setEntityAnimation(self, "PAIN");

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		setCustomAction(self, &helpless, 300, 0, 0);

		if (onGround == 0)
		{
			guardianAddSmokeAlongBody();
		}

		self->action = &guardianAttackFinished;

		for (i=0;i<2;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add the Armour Boss's Star");
			}

			loadProperties("boss/armour_boss_star", e);

			e->x = self->x;
			e->y = self->y;

			e->action = &guardianStarWait;

			e->draw = &drawLoopingAnimationToMap;

			e->thinkTime = 300;

			e->head = self;

			setEntityAnimation(e, "STAND");

			e->currentFrame = (i == 0 ? 0 : 6);

			e->x = self->x + self->w / 2 - e->w / 2;

			e->y = self->y - e->h - 8;
		}
	}

	guardianAlignBodyToHead();
}

static void guardianAddSmokeAlongBody()
{
	int i, bodyLength;

	shakeScreen(MEDIUM, 15);

	bodyLength = abs(self->endX - self->x);

	for (i=0;i<100;i++)
	{
		addSmoke((self->face == LEFT ? self->x : self->endX) + (prand() % bodyLength), self->y + prand() % self->h, "decoration/dust");
	}

	playSoundToMap("sound/common/crash", BOSS_CHANNEL, self->x, self->y, 0);
}

static void guardianBodyTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->head->takeDamage != NULL)
	{
		/* Hitting the body only does half the damage */

		damage /= 2;

		temp = self;

		self = self->head;

		self->takeDamage(other, damage);

		self = temp;
	}
}

static void guardianStarWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0 || self->head->health <= 0)
	{
		self->inUse = FALSE;
	}
}

static void becomeAwesome()
{
	Entity *e, *meter;
	Target *t;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Black Book Awesome Foursome");
	}

	loadProperties("boss/awesome_boss_1", e);

	e->maxHealth = e->health = 200;

	e->flags |= LIMIT_TO_SCREEN;

	e->face = self->face;

	setEntityAnimation(e, "STAND");

	e->x = self->x;
	e->y = self->y;

	e->targetX = e->x;
	e->targetY = getMapFloor(e->x, e->y) - e->h;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->flags &= ~FLY;

	e->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	e->action = &awesomeIntro;

	e->draw = &drawLoopingAnimationToMap;

	e->takeDamage = &awesomeTakeDamage;

	e->die = &awesomeDie;

	e->mental = 2;

	self->flags |= NO_DRAW;

	self->mental = 1;

	self->action = &transformWait;

	meter = addEnemy("boss/awesome_boss_meter", self->x, self->y);

	STRNCPY(meter->objectiveName, "AWESOME_BOSS_METER", sizeof(meter->objectiveName));

	meter->die = &awesomeMeterDie;

	t = getTargetByName("FLY_BOSS_TARGET_TOP_RIGHT");

	if (t == NULL)
	{
		showErrorAndExit("Awesome Boss cannot find target");
	}

	meter->y = t->y;

	meter->head = self;

	e->head = meter;
}

static void awesomeIntro()
{
	char name[MAX_VALUE_LENGTH];
	Entity *e;

	checkToMap(self);

	self->thinkTime--;

	self->endX = 1;

	self->flags |= LIMIT_TO_SCREEN;

	if (self->thinkTime <= 0)
	{
		self->flags |= DO_NOT_PERSIST;

		snprintf(name, sizeof(name), "boss/awesome_boss_%d", self->mental);

		e = addEnemy(name, self->x - 8 * self->mental, self->y - 64);

		e->face = self->face;

		e->active = TRUE;

		e->targetX = e->x;
		e->targetY = e->y;

		e->startX = e->x;
		e->startY = e->y;

		e->maxHealth = e->health = self->maxHealth;

		e->endX = self->mental;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND|LIMIT_TO_SCREEN);

		e->action = &awesomeIntroWait;

		e->takeDamage = &awesomeTakeDamage;

		e->die = &awesomeDie;

		e->head = self;

		playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

		self->thinkTime = 15;

		self->mental++;

		if (self->mental == 5)
		{
			self->target = getEntityByObjectiveName(self->requires);

			if (self->target == NULL)
			{
				showErrorAndExit("Awesome Boss %s cannot find %s", self->objectiveName, self->requires);
			}

			self->thinkTime = 30;

			self->action = &awesomeWait;
		}
	}
}

static void awesomeIntroWait()
{
	if (self->head->mental == 5 && self->head->thinkTime == 0)
	{
		self->target = getEntityByObjectiveName(self->requires);

		if (self->target == NULL)
		{
			showErrorAndExit("Awesome Boss %s cannot find %s", self->objectiveName, self->requires);
		}

		self->action = &awesomeTeleportOut;

		self->touch = &entityTouch;

		initEnergyBar(self);

		self->head = getEntityByObjectiveName("AWESOME_BOSS_METER");

		if (self->head == NULL)
		{
			showErrorAndExit("Awesome Boss could not find meter");
		}

		self->head->damage++;
	}

	checkToMap(self);
}

static void awesomeWait()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			initEnergyBar(self);

			self->head->action = self->head->resumeNormalFunction;

			self->action = &awesomeTeleportOut;

			self->touch = &entityTouch;

			self->head = getEntityByObjectiveName("AWESOME_BOSS_METER");

			if (self->head == NULL)
			{
				showErrorAndExit("Awesome Boss could not find meter");
			}

			self->head->damage++;
		}
	}

	checkToMap(self);
}

static void awesomeTeleportIn()
{
	if (player.health > 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &awesomeAttackFinished;
		}
	}

	checkToMap(self);
}

static void awesomeTeleportOut()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->touch = NULL;

		self->flags |= NO_DRAW;

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &awesomeTeleportWait;

		self->head->mental++;

		#if DEV == 1
		printf("%d / %d : %d / %d\n", self->head->mental, self->head->damage, self->head->health, self->head->maxHealth);
		#endif

		self->thinkTime = 60 + prand() % 120;

		/* Choose if they attack again, or their partner */

		if (self->active == TRUE)
		{
			if (prand() % 2 == 0)
			{
				self->target->thinkTime = 60 + prand() % 120;

				self->target->active = FALSE;
			}

			else
			{
				self->thinkTime = 60 + prand() % 120;

				self->active = FALSE;

				self->target->active = TRUE;
			}
		}
	}

	checkToMap(self);
}

static void awesomeTeleportWait()
{
	int i;
	Target *t;

	setEntityAnimation(self, "STAND");

	if (player.health <= 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->x = self->startX;
			self->y = self->startY;

			self->flags &= ~NO_DRAW;

			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

			playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

			self->action = &awesomeTeleportIn;

			facePlayer();

			self->head->mental--;
		}
	}

	else if (self->target->health <= 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->x = self->target->x;

			t = getTargetByName((prand() % 2 == 0) ? "AWESOME_TARGET_LEFT" : "AWESOME_TARGET_RIGHT");

			if (t == NULL)
			{
				showErrorAndExit("Awesome Boss cannot find target");
			}

			self->y = t->y;

			faceTarget();

			self->thinkTime = 60;

			self->flags &= ~NO_DRAW;

			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

			playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

			self->action = &awesomeHealPartner;

			self->touch = &entityTouch;

			self->dirY = 0;

			self->head->mental--;
		}
	}

	else if (self->head->health == self->head->maxHealth)
	{
		if (self->head->mental == self->head->damage && self->head->targetY <= 0)
		{
			if (self->head->damage == 2)
			{
				t = getTargetByName((int)self->endX % 2 == 0 ? "AWESOME_TARGET_LEFT" : "AWESOME_TARGET_RIGHT");

				self->x = t->x;
				self->y = t->y;

				self->action = &awesomeSuperSpearAttackInit;
			}

			else
			{
				switch (self->head->targetX)
				{
					case 0:
						self->thinkTime = 30 * self->endX;

						self->mental = 5;

						self->action = &awesomeSuperDropAttackInit;

						self->flags |= FLY;

						self->dirY = 0;
					break;

					default:
						t = getTargetByName(self->endX <= 2 ? "AWESOME_TARGET_LEFT" : "AWESOME_TARGET_RIGHT");

						self->x = t->x;
						self->y = t->y;

						self->endY = 1;

						/* The other one will stand behind their partner */

						if (self->endX == 2 || self->endX == 4)
						{
							self->x += self->endX == 2 ? -24 : 24;

							self->endY = 0;
						}

						self->action = &awesomeSuperFireballAttackInit;
					break;
				}
			}
		}
	}

	else if (self->active == TRUE)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			i = prand() % 2;

			switch (i)
			{
				case 0:
					self->action = &awesomeFireballAttackInit;
				break;

				default:
					self->action = &awesomeDropAttackInit;
				break;
			}

			setEntityAnimation(self, "STAND");

			self->dirY = 0;

			self->head->mental--;
		}
	}

	checkToMap(self);
}

static void awesomeFireballAttackInit()
{
	Target *t = getTargetByName((prand() % 2 == 0) ? "AWESOME_TARGET_LEFT" : "AWESOME_TARGET_RIGHT");

	if (t == NULL)
	{
		showErrorAndExit("Awesome Boss cannot find target");
	}

	self->x = t->x + (prand() % 16) * (prand() % 2 == 0 ? 1 : -1);
	self->y = t->y;

	facePlayer();

	playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

	self->flags &= ~NO_DRAW;

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	self->action = &awesomeFireballAttack;

	self->touch = &entityTouch;

	self->thinkTime = -1;
}

static void awesomeFireballAttack()
{
	int i;
	Entity *e;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		setEntityAnimation(self, "ATTACK_1");

		if (self->thinkTime == -1)
		{
			for (i=0;i<6;i++)
			{
				e = getFreeEntity();

				if (e == NULL)
				{
					showErrorAndExit("No free slots to add the Fireball Attack particle");
				}

				loadProperties("boss/awesome_fireball_particle", e);

				setEntityAnimation(e, "STAND");

				e->head = self;

				if (self->face == LEFT)
				{
					e->x = self->x + self->w - e->w - e->offsetX;
				}

				else
				{
					e->x = self->x + e->offsetX;
				}

				e->y = self->y + self->offsetY;

				e->startX = e->x;
				e->startY = e->y;

				e->draw = &drawLoopingAnimationToMap;

				e->mental = 180;

				e->health = i * 60;

				e->action = &awesomeFireballChargeWait;
			}

			self->mental = 6;

			self->thinkTime = 0;
		}

		else if (self->mental <= 0)
		{
			setEntityAnimation(self, "ATTACK_2");

			e = addProjectile("boss/awesome_fireball", self, self->x, self->y, (self->face == RIGHT ? 14 : -14), 0);

			e->touch = &awesomeFireballTouch;

			e->action = &awesomeFireballMove;

			e->type = ENEMY;

			e->x += (self->face == RIGHT ? self->w : e->w);
			e->y += self->offsetY;

			self->thinkTime = 30;

			self->action = &awesomeFireballAttackFinished;

			playSoundToMap("sound/boss/awesome_boss/hadouken", -1, self->x, self->y, 0);
		}
	}
}

static void awesomeAttackFinished()
{
	Entity *temp;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		checkToMap(self);

		temp = self;

		self = self->head;

		self->takeDamage(self, 5);

		self = temp;

		self->action = &awesomeTeleportOut;
	}
}

static void awesomeFireballAttackFinished()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime = 60;

		self->action = &awesomeAttackFinished;
	}
}

static void awesomeSuperFireballAttackInit()
{
	facePlayer();

	playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

	self->flags &= ~NO_DRAW;

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	self->action = &awesomeSuperFireballAttack;

	self->touch = &entityTouch;

	self->thinkTime = -1;

	self->head->mental = 0;
}

static void awesomeSuperFireballAttack()
{
	int i;
	Entity *e;

	checkToMap(self);

	if (self->endY == 1 && (self->flags & ON_GROUND))
	{
		setEntityAnimation(self, "ATTACK_1");

		if (self->thinkTime == -1)
		{
			for (i=0;i<6;i++)
			{
				e = getFreeEntity();

				if (e == NULL)
				{
					showErrorAndExit("No free slots to add the Fireball Attack particle");
				}

				loadProperties("boss/awesome_fireball_particle", e);

				setEntityAnimation(e, "STAND");

				e->head = self;

				if (self->face == LEFT)
				{
					e->x = self->x + self->w - e->w - e->offsetX;
				}

				else
				{
					e->x = self->x + e->offsetX;
				}

				e->y = self->y + self->offsetY;

				e->startX = e->x;
				e->startY = e->y;

				e->draw = &drawLoopingAnimationToMap;

				e->mental = 360;

				e->health = i * 60;

				e->action = &awesomeFireballChargeWait;
			}

			self->mental = 6;

			self->thinkTime = 0;
		}

		else if (self->mental <= 0)
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				setEntityAnimation(self, "ATTACK_2");

				for (i=0;i<5;i++)
				{
					e = addProjectile("boss/awesome_super_fireball", self, self->x, self->y, (self->face == RIGHT ? 14 : -14), 0);

					e->touch = &awesomeFireballTouch;

					e->action = &awesomeFireballMove;

					e->type = ENEMY;

					e->x += (self->face == RIGHT ? self->w : e->w);
					e->y += self->offsetY;

					e->x += self->face == RIGHT ? -2 : 2;
				}

				playSoundToMap("sound/boss/awesome_boss/hadouken", BOSS_CHANNEL, self->x, self->y, 0);

				self->thinkTime = 120;

				self->target->thinkTime = 120;

				self->action = &awesomeSuperFireballAttackFinished;

				self->target->action = &awesomeSuperFireballAttackFinished;
			}
		}
	}

	else if (self->endY == 0 && self->target->health <= 0)
	{
		self->thinkTime = 120;

		self->action = &awesomeTeleportOut;
	}
}

static void awesomeSuperFireballAttackFinished()
{
	self->thinkTime--;

	self->head->health = 0;

	if (self->thinkTime <= 0)
	{
		self->action = &awesomeAttackFinished;
	}

	checkToMap(self);
}

static void awesomeSuperSpearAttackInit()
{
	facePlayer();

	playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

	self->flags &= ~NO_DRAW;

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	self->action = &awesomeSuperSpearAttack;

	self->touch = &entityTouch;

	self->head->mental = 0;

	self->thinkTime = -1;

	if ((int)self->endX % 2 == 1)
	{
		self->endY = 1;
	}
}

static void awesomeSuperSpearAttack()
{
	int i, j;
	Entity *e;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		setEntityAnimation(self, "ATTACK_4");

		if (self->thinkTime == -1)
		{
			for (i=0;i<6;i++)
			{
				e = getFreeEntity();

				if (e == NULL)
				{
					showErrorAndExit("No free slots to add the Fireball Attack particle");
				}

				loadProperties("boss/awesome_fireball_particle", e);

				setEntityAnimation(e, "STAND");

				e->head = self;

				e->x = self->x + self->w / 2 - e->w / 2;
				e->y = self->y + self->h / 2 - e->h / 2;

				e->startX = e->x;
				e->startY = e->y;

				e->draw = &drawLoopingAnimationToMap;

				e->mental = 180;

				e->health = i * 60;

				e->action = &awesomeFireballChargeWait;
			}

			self->mental = 6;

			self->thinkTime = 0;
		}

		else if (self->mental <= 0)
		{
			self->mental = 0;

			setEntityAnimation(self, "ATTACK_3");

			self->thinkTime = 60;

			if (self->face == RIGHT || self->target->health <= 0)
			{
				j = 1;

				for (i=self->x + self->w;i<self->target->x;)
				{
					e = getFreeEntity();

					if (e == NULL)
					{
						showErrorAndExit("No free slots to add a Ground Spear");
					}

					loadProperties("enemy/ground_spear", e);

					setEntityAnimation(e, "STAND");

					e->x = i;
					e->y = self->y + self->h;

					e->startY = e->y - e->h;

					e->endY = e->y;

					e->thinkTime = 15 * j;

					e->damage = 1;

					e->touch = &entityTouch;

					e->action = &awesomeSpearWait;

					e->draw = &drawLoopingAnimationToMap;

					e->head = self;

					i += e->w * 2;

					self->mental++;

					j++;
				}

				self->action = &awesomeSuperSpearAttackFinished;
			}

			else if (self->endY == 0 || self->target->health <= 0)
			{
				j = 1;

				for (i=self->x;i>self->target->x + self->target->w;)
				{
					e = getFreeEntity();

					if (e == NULL)
					{
						showErrorAndExit("No free slots to add a Ground Spear");
					}

					loadProperties("enemy/ground_spear", e);

					setEntityAnimation(e, "STAND");

					e->x = i;
					e->y = self->y + self->h;

					e->startY = e->y - e->h;

					e->endY = e->y;

					e->thinkTime = 15 * j;

					e->damage = 1;

					e->touch = &entityTouch;

					e->action = &awesomeSpearWait;

					e->draw = &drawLoopingAnimationToMap;

					e->head = self;

					i -= e->w * 2;

					self->mental++;

					j++;
				}

				self->action = &awesomeSuperSpearAttackFinished;
			}
		}
	}
}

static void awesomeSuperSpearAttackFinished()
{
	self->head->health = 0;

	if (self->mental <= 0 || self->target->health <= 0)
	{
		self->action = &awesomeAttackFinished;

		self->target->endY = 0;
	}

	checkToMap(self);
}

static void awesomeTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		temp = self;

		self = self->head;

		self->takeDamage(other, damage);

		self = temp;

		if (self->health <= 0)
		{
			self->touch = NULL;

			setEntityAnimation(self, "DIE");

			awesomeAddStunStar();

			self->action = self->die;
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		setCustomAction(self, &flashWhite, 6, 0, 0);

		/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

		if (other->type != PROJECTILE)
		{
			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
		}

		enemyPain();
	}
}

static void awesomeDie()
{
	if (self->target->health <= 0)
	{
		self->head->damage--;

		self->action = &entityDieNoDrop;
	}

	checkToMap(self);
}

static void awesomeHealPartner()
{
	Entity *e;

	if (self->flags & ON_GROUND)
	{
		if (prand() % 2 == 0)
		{
			e = addParticle(self->target->x, self->target->y);

			if (e != NULL)
			{
				e->x += prand() % self->target->w;

				e->y += prand() % self->target->h;

				e->dirY = -(10 + prand() % 11);

				e->dirY /= 10;

				e->thinkTime = 60;
			}
		}

		self->target->health++;

		if (self->target->health >= self->target->maxHealth)
		{
			self->target->health = self->target->maxHealth;

			self->target->action = &awesomeTeleportOut;

			self->action = &awesomeTeleportOut;
		}
	}

	checkToMap(self);
}

static void awesomeDropAttackInit()
{
	int minX, maxX;

	minX = getCameraMinX();
	maxX = getCameraMaxX();

	setEntityAnimation(self, "ATTACK_3");

	self->flags &= ~NO_DRAW;

	self->x = player.x + player.w / 2 - self->w / 2;

	if (self->x < minX)
	{
		self->x = minX;
	}

	else if (self->x + self->w >= maxX)
	{
		self->x = maxX - self->w - 1;
	}

	self->y = self->head->y;

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

	self->action = &awesomeDropAttack;

	self->thinkTime = 60;

	self->dirY = 0;

	self->touch = &entityTouch;

	checkToMap(self);
}

static void awesomeDropAttack()
{
	long onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (onGround == 0)
		{
			addSmokeAlongBody();

			playSoundToMap("sound/enemy/red_grub/thud", -1, self->x, self->y, 0);
		}

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &awesomeAttackFinished;
		}
	}
}

static void awesomeSuperDropAttackInit()
{
	int minX, maxX;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		minX = getCameraMinX();
		maxX = getCameraMaxX();

		setEntityAnimation(self, "ATTACK_3");

		self->flags &= ~(NO_DRAW|FLY);

		self->x = player.x + player.w / 2 - self->w / 2;

		if (self->x < minX)
		{
			self->x = minX;
		}

		else if (self->x + self->w >= maxX)
		{
			self->x = maxX - self->w - 1;
		}

		self->y = self->head->y;

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

		self->touch = &entityTouch;

		self->action = &awesomeSuperDropAttack;

		checkToMap(self);
	}
}

static void awesomeSuperDropAttack()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->mental--;

		if (self->mental <= 0)
		{
			addSmokeAlongBody();

			playSoundToMap("sound/enemy/red_grub/thud", -1, self->x, self->y, 0);

			self->action = &awesomeSuperDropAttackFinished;
		}

		else
		{
			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

			self->flags |= (FLY|NO_DRAW);

			self->thinkTime = 90;

			self->x = player.x + player.w / 2 - self->w / 2;

			self->y = self->head->y;

			self->dirY = 0;

			self->action = &awesomeSuperDropAttackInit;

			self->head->mental = 0;
		}
	}
}

static void awesomeSuperDropAttackFinished()
{
	self->thinkTime = 60;

	self->head->health = 0;

	self->action = &awesomeAttackFinished;

	checkToMap(self);
}

static void awesomeAddStunStar()
{
	int i;
	Entity *e;

	for (i=0;i<2;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Awesome Boss's Star");
		}

		loadProperties("boss/armour_boss_star", e);

		e->x = self->x;
		e->y = self->y;

		e->action = &awesomeStarWait;

		e->draw = &drawLoopingAnimationToMap;

		e->thinkTime = 300;

		e->head = self;

		setEntityAnimation(e, "STAND");

		e->currentFrame = (i == 0 ? 0 : 6);

		e->x = self->x + self->w / 2 - e->w / 2;

		e->y = self->y - e->h;
	}
}

static void awesomeStarWait()
{
	if (self->head->health == self->head->maxHealth || self->head->target->health <= 0)
	{
		self->inUse = FALSE;
	}

	self->x = self->head->x + self->head->w / 2 - self->w / 2;

	self->y = self->head->y - self->h - 8;
}

static void awesomeFireballTouch(Entity *other)
{
	Entity *temp;

	/* Projectiles will cancel each other out */

	if (other->dirX != self->dirX && strcmpignorecase(self->name, other->name) == 0)
	{
		if (self->inUse == FALSE || other->inUse == FALSE)
		{
			return;
		}

		self->inUse = FALSE;

		other->inUse = FALSE;
	}

	else if (self->parent != other && other->takeDamage != NULL)
	{
		temp = self;

		self = other;

		self->takeDamage(temp, self->type == ENEMY ? 50 : temp->damage);

		self = temp;

		self->inUse = FALSE;
	}
}

static void awesomeFireballChargeWait()
{
	float radians;

	self->mental -= 2;

	if (self->mental <= 0)
	{
		self->head->mental--;

		self->inUse = FALSE;
	}

	self->health += 8;

	radians = DEG_TO_RAD(self->health);

	self->x = (0 * cos(radians) - self->mental * sin(radians));
	self->y = (0 * sin(radians) + self->mental * cos(radians));

	self->x += self->startX;
	self->y += self->startY;
}

static void addSmokeAlongBody()
{
	int i;

	for (i=0;i<20;i++)
	{
		addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
	}
}

static void awesomeSpearWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->y == self->startY)
		{
			self->action = &awesomeSpearSink;
		}

		else
		{
			playSoundToMap("sound/enemy/ground_spear/spear", -1, self->x, self->y, 0);

			self->targetY = self->startY;

			self->action = &awesomeSpearRise;
		}
	}
}

static void awesomeSpearSink()
{
	if (self->y < self->endY)
	{
		self->y += self->speed * 2;
	}

	else
	{
		self->inUse = FALSE;

		self->head->mental--;
	}
}

static void awesomeSpearRise()
{
	if (self->y > self->startY)
	{
		self->y -= self->speed * 2;
	}

	else
	{
		self->y = self->startY;

		self->thinkTime = 5;

		self->action = &awesomeSpearWait;
	}
}

static void awesomeFireballMove()
{
	self->thinkTime--;

	if (self->dirX == 0 || self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}

	checkToMap(self);
}

static void awesomeMeterDie()
{
	self->thinkTime--;

	self->takeDamage = NULL;

	if (self->thinkTime <= 0)
	{
		self->targetX = self->head->x + self->head->w / 2 - self->w / 2;
		self->targetY = self->head->y + self->head->h / 2 - self->h / 2;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &transformRemove;
	}

	else
	{
		checkToMap(self);
	}
}

static void becomeBlob()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Black Book Blob");
	}

	loadProperties("boss/blob_boss_2", e);

	e->maxHealth = e->health = 500;

	e->flags |= LIMIT_TO_SCREEN;

	e->face = self->face;

	setEntityAnimation(e, "STAND");

	e->x = self->x;
	e->y = self->y;

	e->targetX = e->x;
	e->targetY = getMapFloor(e->x, e->y) - e->h;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->flags &= ~FLY;

	e->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	e->action = &blobAttackFinished;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &entityTouch;

	e->takeDamage = &blobTakeDamage;

	e->head = self;

	self->flags |= NO_DRAW;

	self->mental = 1;

	self->action = &transformWait;

	initEnergyBar(e);
}

static void blobWait()
{
	int i;

	self->dirX = 0;

	facePlayer();

	self->thinkTime--;

	setEntityAnimation(self, "STAND");

	if (self->thinkTime <= 0 && player.health > 0)
	{
		i = prand() % 3;

		switch (i)
		{
			case 0:
				self->action = &blobPunchAttackInit;
			break;

			case 1:
				self->action = &blobBounceAroundInit;
			break;

			default:
				self->action = &blobGrubAttackInit;
			break;
		}
	}

	checkToMap(self);
}

static void blobPunchAttackInit()
{
	Target *t;

	t = getTargetByName("BLOB_TARGET_LEFT");

	if (t == NULL)
	{
		showErrorAndExit("Blob Boss could not find target");
	}

	self->startX = t->x;

	t = getTargetByName("BLOB_TARGET_RIGHT");

	if (t == NULL)
	{
		showErrorAndExit("Blob Boss could not find target");
	}

	self->endX = t->x;

	self->targetY = self->y + self->h;

	self->maxThinkTime = 3 + prand() % 3;

	self->layer = BACKGROUND_LAYER;

	self->action = &blobPunchSink;
}

static void blobPunchSink()
{
	if (self->y < self->targetY)
	{
		self->y += 3;
	}

	else
	{
		self->y = self->targetY;

		self->flags |= NO_DRAW;

		setEntityAnimation(self, "ATTACK_1");

		if (self->maxThinkTime > 0 && player.health > 0)
		{
			self->action = &blobLookForPlayer;

			self->dirX = self->speed * 1.5;
		}

		else
		{
			self->action = &blobPunchFinish;

			self->targetX = getMapStartX() + SCREEN_WIDTH / 2 - self->w / 2;

			self->targetY = self->y - self->h;

			self->dirX = self->speed;
		}
	}
}

static void blobLookForPlayer()
{
	float target = player.x - self->w / 2 + player.w / 2;

	if (fabs(target - self->x) <= fabs(self->dirX))
	{
		self->targetY = self->y - self->h;

		self->thinkTime = 30;

		self->action = &blobPunch;
	}

	else
	{
		self->x += target > self->x ? self->dirX : -self->dirX;

		if (self->x < self->startX)
		{
			self->x = self->startX;

			self->targetY = self->y - self->h;

			self->thinkTime = 30;

			self->action = &blobPunch;
		}

		else if (self->x > self->endX)
		{
			self->x = self->endX;

			self->targetY = self->y - self->h;

			self->thinkTime = 30;

			self->action = &blobPunch;
		}
	}
}

static void blobPunch()
{
	Entity *e;

	if (self->y > self->targetY)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->thinkTime == 0)
			{
				self->flags &= ~NO_DRAW;

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

			if (self->y <= self->targetY)
			{
				self->y = self->targetY;

				self->maxThinkTime--;

				self->thinkTime = self->maxThinkTime > 0 ? 30 : 90;
			}
		}
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime < 0)
		{
			self->targetY = self->y + self->h;

			self->action = &blobPunchSink;
		}
	}

	facePlayer();
}

static void blobPunchFinish()
{
	if (fabs(self->x - self->targetX) <= fabs(self->dirX))
	{
		self->flags &= ~NO_DRAW;

		setEntityAnimation(self, "STAND");

		if (self->y > self->targetY)
		{
			self->y -= 2;
		}

		else
		{
			self->action = &blobAttackFinished;
		}
	}

	else
	{
		self->x += self->x < self->targetX ? self->dirX : -self->dirX;
	}

	facePlayer();
}

static void blobBounceAroundInit()
{
	self->maxThinkTime = 7;

	self->touch = &entityTouch;

	self->action = &blobBounceAround;

	self->dirY = -16;

	self->face = self->face == LEFT ? RIGHT : LEFT;
}

static void blobBounceAround()
{
	long onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (onGround == 0)
		{
			playSoundToMap("sound/boss/blob_boss/bounce", BOSS_CHANNEL, self->x, self->y, 0);
		}

		self->maxThinkTime--;

		if (self->maxThinkTime > 0)
		{
			self->dirY = -16;
		}

		else
		{
			self->action = &blobAttackFinished;
		}
	}

	if (self->dirX == 0 && self->maxThinkTime != 7)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}
}

static void blobGrubAttackInit()
{
	facePlayer();

	setEntityAnimation(self, "ATTACK_2");

	self->thinkTime = 30;

	self->animationCallback = &blobGrubAttackWait;

	self->maxThinkTime = 1 + prand() % 5;
}

static void blobGrubAttackWait()
{
	setEntityAnimation(self, "ATTACK_3");

	self->thinkTime--;

	self->action = &blobGrubAttackWait;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "ATTACK_4");

		self->action = &blobSpinAttackStart;

		self->thinkTime = 1;
	}
}

static void blobSpinAttackStart()
{
	self->flags |= INVULNERABLE;

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			self->face = (player.x > self->x ? RIGHT : LEFT);

			self->frameSpeed = 2;

			self->dirY = -8;
		}
	}

	else if (self->thinkTime == 0 && self->flags & ON_GROUND)
	{
		self->speed = self->originalSpeed * 6;

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

		self->action = &blobSpinAttack;

		self->thinkTime = 180;

		self->flags |= ATTACKING;
	}

	checkToMap(self);
}

static void blobSpinAttack()
{
	self->thinkTime--;

	checkToMap(self);

	if (self->dirX == 0 || isAtEdge(self))
	{
		shakeScreen(MEDIUM, 15);

		self->dirX = self->face == LEFT ? 3 : -3;

		self->dirY = -6;

		self->action = &blobSpinAttackEnd;

		self->thinkTime = 0;

		playSoundToMap("sound/common/crash", -1, self->x, self->y, 0);

		facePlayer();
	}

	else if (self->thinkTime <= 0)
	{
		self->action = &blobSpinAttackEnd;

		self->thinkTime = 0;
	}
}

static void blobSpinAttackEnd()
{
	checkToMap(self);

	if ((self->flags & ON_GROUND) && self->thinkTime == 0)
	{
		facePlayer();

		self->dirX = 0;

		self->maxThinkTime--;

		if (self->maxThinkTime > 0)
		{
			self->action = &blobSpinAttackStart;

			self->thinkTime = 0;
		}

		else
		{
			self->action = &blobGrubAttackFinish;

			self->thinkTime = 30;
		}
	}
}

static void blobGrubAttackFinish()
{
	if (self->frameSpeed > 0)
	{
		self->frameSpeed = -1;

		facePlayer();

		setEntityAnimation(self, "ATTACK_2");

		self->animationCallback = &blobAttackFinished;

		self->frameSpeed = 0;
	}

	else if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->frameSpeed = -1;
		}
	}
}

static void blobAttackFinished()
{
	self->flags &= ~INVULNERABLE;

	self->layer = MID_GROUND_LAYER;

	self->frameSpeed = 1;

	setEntityAnimation(self, "STAND");

	self->speed = self->originalSpeed;

	self->dirX = 0;

	self->thinkTime = 90;

	self->damage = 1;

	self->action = &blobWait;

	self->touch = &entityTouch;

	self->activate = NULL;
}

static void blobTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		self->health -= damage;

		if (self->health > 0)
		{
			enemyPain();
		}

		else
		{
			self->frameSpeed = 0;

			self->animationCallback = NULL;

			self->thinkTime = 120;

			self->startX = self->x;

			self->damage = 0;

			self->action = &blobDie;
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

static void blobDie()
{
	self->thinkTime--;

	self->takeDamage = NULL;

	if (self->thinkTime <= 0)
	{
		self->targetX = self->head->x + self->head->w / 2 - self->w / 2;
		self->targetY = self->head->y + self->head->h / 2 - self->h / 2;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &transformRemove;
	}

	else
	{
		blobShudder();

		checkToMap(self);
	}
}

static void blobShudder()
{
	self->startY += 90;

	if (self->startY >= 360)
	{
		self->startY = 0;
	}

	self->x = self->startX + sin(DEG_TO_RAD(self->startY)) * 4;
}

static void becomeQueenWasp()
{
	Entity *e = getFreeEntity();
	Target *t;

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Black Book Queen Wasp");
	}

	loadProperties("boss/fly_boss", e);

	e->maxHealth = e->health = 500;

	e->flags |= LIMIT_TO_SCREEN;

	e->face = self->face;

	setEntityAnimation(e, "STAND");

	t = getTargetByName("FLY_BOSS_TARGET_TOP_RIGHT");

	if (t == NULL)
	{
		showErrorAndExit("Fly boss cannot find target");
	}

	e->x = self->x;
	e->y = self->y;

	e->targetX = t->x;
	e->targetY = t->y;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	e->action = &queenWaspAttackFinished;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &entityTouch;

	e->takeDamage = &queenWaspTakeDamage;

	e->head = self;

	self->flags |= NO_DRAW;

	self->mental = 1;

	self->action = &transformWait;

	initEnergyBar(e);
}

static void queenWaspWait()
{
	int i;

	self->thinkTime--;

	facePlayer();

	queenWaspHover();

	if (self->thinkTime <= 0 && player.health > 0)
	{
		i = prand() % 4;

		switch (i)
		{
			case 0:
				self->action = &queenWaspBulletFireInit;
			break;

			case 1:
				self->action = &queenWaspHeadButtInit;
			break;

			case 2:
				self->thinkTime = 120 + prand() % 180;

				self->action = &queenWaspDropInit;
			break;

			default:
				self->action = &queenWaspSlimeFireInit;
			break;
		}

		self->damage = 1;

		playSoundToMap("sound/boss/fly_boss/buzz", BOSS_CHANNEL, self->x, self->y, 0);
	}
}

static void queenWaspHover()
{
	self->startX++;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 8;
}

static void queenWaspFlyToTopTarget()
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

	self->action = &queenWaspMoveToTarget;
}

static void queenWaspMoveToTarget()
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

		self->action = &queenWaspWait;
	}
}

static void queenWaspDropInit()
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

		self->action = &queenWaspDrop;

		self->dirX = 0;
	}
}

static void queenWaspDrop()
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

			self->action = &queenWaspDropWait;

			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}
		}
	}
}

static void queenWaspDropWait()
{
	self->thinkTime--;

	facePlayer();

	if (self->thinkTime <= 0)
	{
		self->flags |= FLY;

		self->action = &queenWaspAttackFinished;
	}
}

static void queenWaspBulletFireInit()
{
	queenWaspSelectRandomBottomTarget();

	self->action = &queenWaspBulletFireMoveToPosition;
}

static void queenWaspBulletFireMoveToPosition()
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

		self->action = &queenWaspFireBullets;
	}
}

static void queenWaspFireBullets()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime--;

		playSoundToMap("sound/boss/fly_boss/fly_boss_bullet", BOSS_CHANNEL, self->x, self->y, 0);

		e = addProjectile("boss/fly_boss_shot", self, self->x + (self->face == RIGHT ? self->w : 0), self->y + self->h / 2, (self->face == RIGHT ? 7 : -7), 0);

		e->dirY = 0.1 * (prand() % 2 == 0 ? -1 : 1);

		e->reactToBlock = &bounceOffShield;

		if (self->maxThinkTime <= 0)
		{
			self->thinkTime = 120;

			self->action = &queenWaspAttackFinished;
		}

		else
		{
			self->thinkTime = 6;
		}
	}
}

static void queenWaspSlimeFireInit()
{
	queenWaspSelectRandomBottomTarget();

	self->action = &queenWaspSlimeFireMoveToPosition;
}

static void queenWaspSlimeFireMoveToPosition()
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

		self->action = &queenWaspFireSlime;
	}
}

static void queenWaspFireSlime()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime--;

		playSoundToMap("sound/boss/grub_boss/fire", BOSS_CHANNEL, self->x, self->y, 0);

		e = addProjectile("boss/fly_boss_slime", self, self->x + (self->face == RIGHT ? self->w : 0), self->y + self->h / 2, (self->face == RIGHT ? 7 : -7), 0);

		e->touch = &queenWaspSlimePlayer;

		if (self->maxThinkTime <= 0)
		{
			self->thinkTime = 60;

			self->action = ((player.flags & HELPLESS) || prand() % 3 == 0) ? &queenWaspBulletFireMoveToPosition : &queenWaspAttackFinished;
		}

		else
		{
			self->thinkTime = 30;
		}
	}
}

static void queenWaspHeadButtInit()
{
	queenWaspSelectRandomBottomTarget();

	self->action = &queenWaspHeadButtMoveToPosition;
}

static void queenWaspHeadButtMoveToPosition()
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

		self->action = &queenWaspMoveToHeadButtRange;
	}
}

static void queenWaspMoveToHeadButtRange()
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

			self->action = &queenWaspHeadButt;

			self->reactToBlock = &queenWaspReactToHeadButtBlock;
		}

		else
		{
			self->dirX = self->face == LEFT ? -self->speed : self->speed;
		}
	}

	checkToMap(self);
}

static void queenWaspHeadButt()
{
	self->touch = &queenWaspRamTouch;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->dirX = (self->face == LEFT ? 4 : -4);
		self->dirY = -6;

		self->action = &queenWaspHeadButtFinish;
	}
}

static void queenWaspHeadButtFinish()
{
	self->touch = &entityTouch;

	if (self->flags & ON_GROUND)
	{
		facePlayer();

		self->dirX = 0;

		self->thinkTime = 120;

		if (prand() % 2 == 0)
		{
			self->action = &queenWaspAttackFinished;
		}

		else
		{
			self->action = &queenWaspMoveToHeadButtRange;
		}
	}

	checkToMap(self);
}

static void queenWaspSelectRandomBottomTarget()
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

static void queenWaspReactToHeadButtBlock(Entity *other)
{
	self->dirX = 0;
}

static void queenWaspTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
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
			self->health = 0;

			self->thinkTime = 180;

			self->flags &= ~FLY;

			self->frameSpeed = 0;

			self->takeDamage = NULL;

			self->touch = NULL;

			self->action = &queenWaspDie;

			self->startX = self->x;
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

static void queenWaspDie()
{
	self->thinkTime--;

	self->takeDamage = NULL;

	if (self->thinkTime <= 0)
	{
		self->targetX = self->head->x + self->head->w / 2 - self->w / 2;
		self->targetY = self->head->y + self->head->h / 2 - self->h / 2;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &transformRemove;
	}

	else
	{
		queenWaspShudder();

		checkToMap(self);
	}
}

static void queenWaspShudder()
{
	self->startY += 90;

	if (self->startY >= 360)
	{
		self->startY = 0;
	}

	self->x = self->startX + sin(DEG_TO_RAD(self->startY)) * 4;
}

static void queenWaspAttackFinished()
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

		self->action = &queenWaspFlyToTopTarget;

		self->reactToBlock = NULL;

		self->thinkTime = 30;
	}

	checkToMap(self);
}

static void queenWaspRamTouch(Entity *other)
{
	int health = player.health;

	entityTouch(other);

	if (player.health < health)
	{
		queenWaspReactToHeadButtBlock(other);
	}
}

static void queenWaspSlimePlayer(Entity *other)
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

static void becomeKingGrub()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Black Book King Grub");
	}

	loadProperties("boss/grub_boss", e);

	e->maxHealth = e->health = 500;

	e->flags |= LIMIT_TO_SCREEN;

	e->face = self->face;

	setEntityAnimation(e, "STAND");

	e->x = self->x;
	e->y = self->y;

	e->targetX = e->x;
	e->targetY = getMapFloor(e->x, e->y) - e->h;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	e->action = &kingGrubAttackFinished;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &entityTouch;

	e->takeDamage = &kingGrubTakeDamage;

	e->head = self;

	self->flags |= NO_DRAW;

	self->mental = 1;

	self->action = &transformWait;

	initEnergyBar(e);
}

static void kingGrubWait()
{
	int attack;

	self->dirX = 0;

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		attack = prand() % 3;

		switch (attack)
		{
			case 0:
				self->action = &kingGrubSpitStart;

				self->thinkTime = (prand() % 3) + 1;
			break;

			case 1:
				self->action = &kingGrubSpinAttackStart;

				self->thinkTime = 60;
			break;

			default:
				self->action = &kingGrubBounceAttackStart;

				self->thinkTime = 60;
			break;
		}
	}

	checkToMap(self);
}

static void kingGrubSpitStart()
{
	if (self->frameSpeed > 0)
	{
		if (self->thinkTime > 0)
		{
			setEntityAnimation(self, "ATTACK_1");

			self->animationCallback = &kingGrubSpit;
		}

		else
		{
			self->action = &kingGrubAttackFinished;
		}
	}

	else
	{
		self->animationCallback = &kingGrubSpitEnd;
	}

	checkToMap(self);
}

static void kingGrubSpitEnd()
{
	self->frameSpeed *= -1;
}

static void kingGrubSpit()
{
	int x = (self->face == RIGHT ? self->offsetX : self->w - self->offsetX);
	int y = self->offsetY;

	addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + y, (self->face == RIGHT ? 7 : -7), -12);
	addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + y, (self->face == RIGHT ? 4 : -4), -12);
	addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + y, (self->face == RIGHT ? 1 : -1), -12);
	addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + y, (self->face == RIGHT ? 2.5 : -2.5), -12);
	addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + y, (self->face == RIGHT ? 5.5 : -5.5), -12);

	playSoundToMap("sound/boss/grub_boss/fire", BOSS_CHANNEL, self->x, self->y, 0);

	self->thinkTime--;

	self->frameSpeed *= -1;
}

static void kingGrubSpinAttackStart()
{
	self->flags |= INVULNERABLE;

	setEntityAnimation(self, "ATTACK_2");

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			self->face = (player.x > self->x ? RIGHT : LEFT);

			self->frameSpeed = 2;

			self->dirY = -8;
		}
	}

	else if (self->thinkTime == 0 && self->flags & ON_GROUND)
	{
		shakeScreen(MEDIUM, 15);

		playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

		self->action = &kingGrubSpinAttack;
	}

	checkToMap(self);
}

static void kingGrubSpinAttack()
{
	float speed = self->dirX;

	self->flags |= INVULNERABLE;

	checkToMap(self);

	if (self->dirX == 0)
	{
		shakeScreen(MEDIUM, 15);

		playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);

		self->face = (player.x > self->x ? RIGHT : LEFT);

		self->dirX = speed < 0 ? 3 : -3;

		self->dirY = -6;

		self->action = &kingGrubSpinAttackEnd;

		self->thinkTime = 0;
	}
}

static void kingGrubSpinAttackEnd()
{
	checkToMap(self);

	if ((self->flags & ON_GROUND) && self->thinkTime == 0)
	{
		self->dirX = 0;

		self->action = &kingGrubAttackFinished;
	}
}

static void kingGrubBounceAttackStart()
{
	self->flags |= INVULNERABLE;

	setEntityAnimation(self, "ATTACK_2");

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			self->face = (player.x > self->x ? RIGHT : LEFT);

			self->frameSpeed = 2;

			self->dirY = -8;
		}
	}

	else if (self->thinkTime == 0 && self->flags & ON_GROUND)
	{
		shakeScreen(MEDIUM, 15);

		playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);

		self->dirX = (self->face == RIGHT ? 3 : -3);

		self->dirY = -14;

		self->action = &kingGrubBounceAttack;
	}

	checkToMap(self);
}

static void kingGrubBounceAttack()
{
	float speed = self->dirX;

	self->flags |= INVULNERABLE;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		shakeScreen(MEDIUM, 15);

		playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);

		self->dirY = -14;
	}

	if (self->dirX == 0)
	{
		shakeScreen(MEDIUM, 15);

		playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);

		self->face = (player.x > self->x ? RIGHT : LEFT);

		self->dirX = speed < 0 ? 3 : -3;

		self->dirY = -6;

		self->action = &kingGrubBounceAttackEnd;

		self->thinkTime = 0;
	}
}

static void kingGrubBounceAttackEnd()
{
	checkToMap(self);

	if ((self->flags & ON_GROUND) && self->thinkTime == 0)
	{
		self->dirX = 0;

		self->action = &kingGrubAttackFinished;
	}
}

static void kingGrubAttackFinished()
{
	self->flags &= ~INVULNERABLE;

	setEntityAnimation(self, "STAND");

	self->frameSpeed = 1;

	self->thinkTime = 90;

	self->action = &kingGrubWait;
}

static void kingGrubTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
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
			self->health = 0;

			self->thinkTime = 180;

			self->flags &= ~FLY;

			setEntityAnimation(self, "STAND");

			self->frameSpeed = 0;

			self->takeDamage = NULL;

			self->touch = NULL;

			self->action = &kingGrubDie;

			self->startX = self->x;

			playSoundToMap("sound/boss/grub_boss/death", BOSS_CHANNEL, self->x, self->y, 0);
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

static void kingGrubDie()
{
	self->thinkTime--;

	self->takeDamage = NULL;

	if (self->thinkTime <= 0)
	{
		self->targetX = self->head->x + self->head->w / 2 - self->w / 2;
		self->targetY = self->head->y + self->head->h / 2 - self->h / 2;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &transformRemove;
	}

	else
	{
		kingGrubShudder();

		checkToMap(self);
	}
}

static void kingGrubShudder()
{
	self->startY += 90;

	if (self->startY >= 360)
	{
		self->startY = 0;
	}

	self->x = self->startX + sin(DEG_TO_RAD(self->startY)) * 4;
}

static void blackBookAttackFinished()
{
	self->thinkTime = 120;

	self->action = &blackBookWait;

	hover();
}

static void transformWait()
{
	if (self->mental == 0)
	{
		self->health -= 500;

		self->flags &= ~NO_DRAW;

		if (self->health <= 0)
		{
			self->startX = self->x;

			self->thinkTime = 300;

			self->action = &blackBookDie;
		}

		else
		{
			self->action = &blackBookAttackFinished;
		}
	}
}

static void blackBookDie()
{
	Entity *e;

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			e = addEnemy("boss/black_book_3", self->x, self->y);

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			e->active = FALSE;

			clearContinuePoint();

			increaseKillCount();

			freeBossHealthBar();

			self->flags |= NO_DRAW;

			self->thinkTime = 180;

			self->action = &dieFinish;
		}

		else
		{
			blackBookShudder();
		}
	}

	checkToMap(self);
}

static void dieFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &entityDieVanish;
	}
}

static void blackBookShudder()
{
	Entity *e;

	self->startY += 90;

	if (self->startY >= 360)
	{
		self->startY = 0;
	}

	self->x = self->startX + sin(DEG_TO_RAD(self->startY)) * 4;

	if (self->thinkTime <= 180 && (self->thinkTime % 10) == 0)
	{
		e = addTemporaryItem("boss/black_book_page", self->x, self->y, self->face, 0, 0);

		playSoundToMap("sound/boss/black_book/page", -1, self->x, self->y, 0);

		e->targetX = self->x + SCREEN_WIDTH;
		e->targetY = self->y + (prand() % 96) * (prand() % 2 == 0 ? -1 : 1);

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->speed = 20 + (prand() % 40);

		e->speed /= 10;

		e->dirX *= e->speed;
		e->dirY *= e->speed;
	}
}

static void transformRemove()
{
	self->head->mental = 0;

	self->inUse = FALSE;
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

static void backWait()
{
	self->face = self->head->face;

	self->x = self->head->x;
	self->y = self->head->y;

	if (self->head->flags & NO_DRAW)
	{
		self->flags |= NO_DRAW;
	}

	else
	{
		self->flags &= ~NO_DRAW;
	}

	if (!(self->head->flags & FLY) || self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static void initEnergyBar(Entity *boss)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Awesome Boss Energy Bar");
	}

	loadProperties("boss/awesome_boss_energy_bar", e);

	e->action = &energyBarWait;

	e->draw = &energyBarDraw;

	e->type = ENEMY;

	e->active = FALSE;

	e->head = boss;

	setEntityAnimation(e, "STAND");
}

static void energyBarWait()
{
	int increment;

	self->x = self->head->x - (self->w - self->head->w) / 2;
	self->y = self->head->y - self->h - 4;

	if (self->health < self->head->health)
	{
		increment = self->head->health / 100;

		if (increment < 1)
		{
			increment = 1;
		}

		self->health += increment;

		if (self->health > self->head->health)
		{
			self->health = self->head->health;
		}
	}

	else if (self->head->health < self->health)
	{
		self->health -= 3;

		if (self->health < self->head->health)
		{
			self->health = self->head->health;
		}
	}

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}

	self->layer = self->head->layer;
}

static int energyBarDraw()
{
	int width;
	float percentage;

	if (!(self->head->flags & NO_DRAW))
	{
		drawLoopingAnimationToMap();

		percentage = self->health;
		percentage /= self->head->maxHealth;

		width = self->w - 2;

		width *= percentage;

		if (percentage >= 0.5)
		{
			drawBoxToMap(self->x + 1, self->y + 1, width, self->h - 2, 0, 220, 0);
		}

		else if (percentage >= 0.25)
		{
			drawBoxToMap(self->x + 1, self->y + 1, width, self->h - 2, 220, 220, 0);
		}

		else if (percentage > 0)
		{
			drawBoxToMap(self->x + 1, self->y + 1, width, self->h - 2, 220, 0, 0);
		}
	}

	return TRUE;
}

static void creditsMove()
{
	addBack();

	self->creditsAction = &bossMoveToMiddle;
}

static void addExitTrigger(Entity *e)
{
	char itemName[MAX_LINE_LENGTH];

	snprintf(itemName, MAX_LINE_LENGTH, "\"%s\" 1 UPDATE_EXIT \"BLACK_BOOK_2\"", e->objectiveName);

	addGlobalTriggerFromScript(itemName);
}
