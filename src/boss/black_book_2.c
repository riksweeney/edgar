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
#include "../system/random.h"
#include "../audio/audio.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "../map.h"
#include "../projectile.h"
#include "../graphics/decoration.h"
#include "../graphics/graphics.h"
#include "../game.h"
#include "../audio/music.h"
#include "../graphics/gib.h"
#include "../event/trigger.h"
#include "../hud.h"
#include "../inventory.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../world/target.h"
#include "../system/error.h"
#include "../geometry.h"
#include "../enemy/enemies.h"
#include "../event/script.h"
#include "../player.h"
#include "../enemy/rock.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void blackBookAttackFinished(void);
static void blackBookWait(void);
static void hover(void);
static void becomeKingGrub(void);
static void becomeQueenWasp(void);
static void becomeBlob(void);
static void transformWait(void);
static void transformRemove(void);
static int energyBarDraw(void);
static void energyBarWait(void);
static void initEnergyBar(Entity *);
static void addSmokeAlongBody(void);
static void blackBookDie(void);
static void blackBookShudder(void);

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
static void queenWaspSelectRandomBottomTarget(void);
static void queenWaspReactToHeadButtBlock(Entity *);
static void queenWaspDropInit(void);
static void queenWaspDrop(void);
static void queenWaspDropWait(void);
static void queenWaspRamTouch(Entity *);
static void queenWaspDie(void);
static void queenWaspShudder(void);

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

static void becomeGolem(void);
static void golemHeadReform(void);
static void golemHeadReform2(void);
static void golemReform(void);
static void golemReform2(void);
static void golemInitialShatter(void);
static void golemDie(void);
static void golemTakeDamage(Entity *, int);
static void golemShatter(void);
static void golemReform(void);
static void golemHeadReform(void);
static void golemReform2(void);
static void golemHeadReform2(void);
static void golemWait(void);
static void golemHeadWait(void);
static void golemPartWait(void);
static void golemAttackFinished(void);
static void golemStompAttackStart(void);
static void golemStompAttack(void);
static void golemStompShake(void);
static void golemStompAttackFinish(void);
static void golemAttackFinished(void);
static void golemStunnedTouch(Entity *);
static void golemThrowRockStart(void);
static void golemThrowRock(void);
static void golemThrowRockFinish(void);
static void golemRockWait(void);
static void golemJumpAttackStart(void);
static void golemJumpAttack(void);
static void golemRockTouch(Entity *);

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

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void initialise()
{
	int minX, minY;

	if (self->active == TRUE)
	{
		minX = getMapStartX();
		minY = getMapStartY();

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

		self->targetX = self->startX;
		self->targetY = self->startY;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

		self->thinkTime = 120;

		self->action = &introPause;
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

static void introPause()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		initBossHealthBar();

		playDefaultBossMusic();

		self->action = &blackBookAttackFinished;
	}

	hover();
}

static void blackBookWait()
{
	int i;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		i = prand() % 6;

		switch (i)
		{
			case 0:
				self->action = &becomeKingGrub;
			break;

			case 1:
				self->action = &becomeQueenWasp;
			break;

			case 2:
				self->action = &becomeBlob;
			break;

			case 3:
				self->action = &becomeGolem;
			break;

			case 4:
				self->action = &becomeGuardian;
			break;

			default:
				self->action = &becomeAwesome;
			break;
		}
	}

	hover();
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
		showErrorAndExit("Snake boss is missing target");
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
		playSoundToMap("sound/boss/snake_boss/hiss.ogg", BOSS_CHANNEL, self->x, self->y, 0);
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

	body = (Entity **)malloc(self->mental * sizeof(Entity *));

	if (body == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Snake Boss body...", self->mental * (int)sizeof(Entity *));
	}

	snprintf(bodyName, sizeof(bodyName), "%s_body", self->name);

	/* Create in reverse order so that it is drawn correctly */

	resetEntityIndex();

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
	}

	guardianAlignBodyToHead();
}

static void guardianBiteAttack()
{
	self->dirX = (self->targetX < self->x ? -self->speed * 2 : self->speed * 2);

	checkToMap(self);

	if (fabs(self->x - self->targetX) < self->speed)
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
			showErrorAndExit("Snake boss is missing target");
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

		addDamageScore(damage, self);
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

	if (self->thinkTime <= 0)
	{
		self->flags &= ~FLY;

		self->dirX = self->face == LEFT ? -8 : 8;

		setEntityAnimation(self, "DIE");
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
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

	playSoundToMap("sound/common/crash.ogg", BOSS_CHANNEL, self->x, self->y, 0);
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

static void becomeGolem()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Black Book Golem");
	}

	loadProperties("boss/golem_boss", e);

	e->maxHealth = e->health = 50;

	e->flags |= LIMIT_TO_SCREEN;

	e->face = self->face;

	setEntityAnimation(e, "STAND");

	e->x = self->x;
	e->y = self->y;

	e->targetX = e->x;
	e->targetY = getMapFloor(e->x, e->y) - e->h;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

	e->action = &golemInitialShatter;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &entityTouch;

	e->takeDamage = &golemTakeDamage;

	e->head = self;

	self->flags |= NO_DRAW;

	self->mental = 1;

	self->action = &transformWait;
}

static void golemInitialShatter()
{
	Entity *e;
	Target *t;

	e = getEntityByObjectiveName("GOLEM_ROCK_DROPPER");

	if (e == NULL)
	{
		e = addEnemy("boss/golem_rock_dropper", self->x, self->y);

		STRNCPY(e->objectiveName, "GOLEM_ROCK_DROPPER", sizeof(e->objectiveName));

		t = getTargetByName("ROCK_DROPPER_TARGET");

		if (t == NULL)
		{
			showErrorAndExit("Awesome Boss cannot find target");
		}

		e->x = t->x;
		e->y = t->y;
	}

	self->startY = self->y;

	self->action = &golemAttackFinished;

	initEnergyBar(self);
}

static void golemShatter()
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

	setEntityAnimation(self, "CUSTOM_1");

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

		e->flags |= LIMIT_TO_SCREEN;

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

		setEntityAnimationByID(e, i);

		e->action = &golemPartWait;

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

	self->touch = &golemStunnedTouch;

	self->thinkTime = 300;

	self->action = &golemHeadWait;
}

static void golemWait()
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

				self->action = &golemThrowRockStart;
			break;

			case 1:
				self->action = &golemStompAttackStart;
			break;

			default:
				self->action = &golemJumpAttackStart;
			break;
		}
	}

	facePlayer();

	checkToMap(self);
}

static void golemStompAttackStart()
{
	setEntityAnimation(self, "ATTACK_1");

	self->dirX = 0;

	self->action = &golemStompAttack;

	self->animationCallback = &golemStompShake;

	checkToMap(self);
}

static void golemStompAttack()
{
	checkToMap(self);
}

static void golemStompShake()
{
	setEntityAnimation(self, "ATTACK_2");

	playSoundToMap("sound/common/crash.ogg", BOSS_CHANNEL, self->x, self->y, 0);

	shakeScreen(STRONG, 120);

	activateEntitiesValueWithObjectiveName("GOLEM_ROCK_DROPPER", 5);

	self->frameSpeed = 0;

	self->thinkTime = 120;

	if (player.flags & ON_GROUND)
	{
		setPlayerStunned(120);
	}

	self->action = &golemStompAttackFinish;
}

static void golemStompAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &golemAttackFinished;
	}

	checkToMap(self);
}

static void golemAttackFinished()
{
	self->frameSpeed = 1;

	setEntityAnimation(self, "STAND");

	self->thinkTime = 60;

	self->action = &golemWait;

	checkToMap(self);
}

static void golemStunnedTouch(Entity *other)
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

static void golemTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

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
			golemShatter();

			self->thinkTime = 120;

			self->startX = self->x;

			self->damage = 0;

			self->action = &golemDie;
		}
	}

	else
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

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		damage = 0;
	}

	addDamageScore(damage, self);
}

static void golemDie()
{
	int i;
	long onGround;
	Entity *e;

	onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (onGround == 0)
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

			self->thinkTime = 120;
			
			playSoundToMap("sound/common/crumble.ogg", BOSS_CHANNEL, self->x, self->y, 0);
			
			self->flags |= NO_DRAW;
		}

		else
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				e = self->target;

				while (e != NULL)
				{
					e->thinkTime = 60 + prand() % 120;

					e->action = &generalItemAction;

					e = e->target;
				}

				self->targetX = self->head->x + self->head->w / 2 - self->w / 2;
				self->targetY = self->head->y + self->head->h / 2 - self->h / 2;

				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

				self->flags |= (NO_DRAW|HELPLESS|TELEPORTING);

				self->action = &transformRemove;
			}
		}
	}
}

static void golemThrowRockStart()
{
	/* Crouch to pick up rock */

	setEntityAnimation(self, "ATTACK_3");

	self->animationCallback = &golemThrowRock;

	checkToMap(self);
}

static void golemThrowRock()
{
	Entity *e;

	e = addEnemy("enemy/small_boulder", self->x, self->y);

	e->x += self->face == RIGHT ? self->w : -e->w;

	e->dirX = 0;
	e->dirY = 0;

	e->face = RIGHT;

	e->frameSpeed = 0;

	e->flags |= FLY;

	e->action = &golemRockWait;

	e->flags &= ~FLY;

	e->dirX = self->face == LEFT ? -2 * (1 + prand() % 5) : 2 * (1 + prand() % 5);

	e->dirY = -10;

	e->touch = &golemRockTouch;

	setEntityAnimation(self, "ATTACK_4");

	self->thinkTime = 60;

	self->action = &golemThrowRockFinish;
}

static void golemThrowRockFinish()
{
	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		self->maxThinkTime--;

		if (self->maxThinkTime <= 0)
		{
			self->action = &golemAttackFinished;
		}

		else
		{
			setEntityAnimation(self, "ATTACK_5");

			self->animationCallback = &golemThrowRock;
		}
	}

	checkToMap(self);
}

static void golemJumpAttackStart()
{
	self->maxThinkTime = 4;

	/* First jump is on the spot */

	self->dirY =- 13;

	self->dirX = 0;

	self->action = &golemJumpAttack;

	self->thinkTime = 30;

	checkToMap(self);
}

static void golemJumpAttack()
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
				self->action = &golemAttackFinished;
			}
		}

		else
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->dirY =- 13;

				self->dirX = self->face == LEFT ? -6 : 6;

				self->action = &golemJumpAttack;
			}
		}
	}
}

static void golemReform()
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

			self->action = &golemPartWait;
		}

		else
		{
			self->dirX = self->targetX > self->x ? self->speed : -self->speed;
		}
	}

	checkToMap(self);
}

static void golemHeadReform()
{
	Entity *e;
	float speed;

	if (self->maxThinkTime == 0)
	{
		self->action = &golemHeadReform2;

		self->flags |= FLY;

		speed = 5 + prand() % 15;

		speed /= 10;

		self->dirY = -speed;

		e = self->target;

		while (e != NULL)
		{
			self->maxThinkTime++;

			e->action = &golemReform2;

			e->flags |= FLY;

			speed = 5 + prand() % 15;

			speed /= 10;

			e->dirY = -speed;

			e = e->target;
		}
	}

	checkToMap(self);
}

static void golemReform2()
{
	if (self->y <= self->targetY)
	{
		self->dirY = 0;

		self->y = self->targetY;

		self->head->maxThinkTime--;

		self->action = &golemPartWait;
	}

	checkToMap(self);
}

static void golemHeadReform2()
{
	Entity *e;

	if (self->y <= self->targetY)
	{
		self->dirY = 0;

		self->y = self->targetY;

		if (self->maxThinkTime == 0)
		{
			self->thinkTime = 90;

			self->action = &golemWait;

			setEntityAnimation(self, "STAND");

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

static void golemPartWait()
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

static void golemHeadWait()
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

			self->action = &golemHeadReform;

			e = self->target;

			while (e != NULL)
			{
				e->action = &golemReform;

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

static void golemRockWait()
{
	float dirX = self->dirX;

	checkToMap(self);

	if ((self->flags & ON_GROUND) || (self->dirX == 0 && dirX != 0 && !(self->flags & ON_GROUND)))
	{
		self->die();
	}
}

static void golemRockTouch(Entity *other)
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

		playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

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

		playSoundToMap("sound/common/spell.ogg", BOSS_CHANNEL, self->x, self->y, 0);

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

			playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

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

			playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

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

	playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

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

			playSoundToMap("sound/boss/awesome_boss/hadouken.ogg", -1, self->x, self->y, 0);
		}
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

	playSoundToMap("sound/common/spell.ogg", BOSS_CHANNEL, self->x, self->y, 0);

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

				playSoundToMap("sound/boss/awesome_boss/hadouken.ogg", BOSS_CHANNEL, self->x, self->y, 0);

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

	playSoundToMap("sound/common/spell.ogg", BOSS_CHANNEL, self->x, self->y, 0);

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

			if (self->face == RIGHT)
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

			else if (self->endY == 0)
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

	if (self->mental <= 0)
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

		addDamageScore(damage, self);
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

	playSoundToMap("sound/common/spell.ogg", -1, self->x, self->y, 0);

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

			playSoundToMap("sound/enemy/red_grub/thud.ogg", -1, self->x, self->y, 0);
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

		playSoundToMap("sound/common/spell.ogg", BOSS_CHANNEL, self->x, self->y, 0);

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

			playSoundToMap("sound/enemy/red_grub/thud.ogg", -1, self->x, self->y, 0);

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
			playSoundToMap("sound/enemy/ground_spear/spear.ogg", -1, self->x, self->y, 0);

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
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
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

		playSoundToMap("sound/common/crash.ogg", -1, self->x, self->y, 0);

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

		addDamageScore(damage, self);
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
		showErrorAndExit("Fly boss is missing target");
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

		playSoundToMap("sound/boss/fly_boss/buzz.ogg", BOSS_CHANNEL, self->x, self->y, 0);
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
		showErrorAndExit("Fly boss is missing target");
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
	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		self->damage = 1;
	}

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
			playSoundToMap("sound/common/crash.ogg", BOSS_CHANNEL, self->x, self->y, 0);

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
			self->dirX = 0;

			self->touch = &queenWaspRamTouch;

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
	facePlayer();

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
	}

	self->dirX = self->face == LEFT ? -self->speed * 3 : self->speed * 3;
	self->dirY = -3;

	checkToMap(self);

	self->thinkTime = 120;

	if (prand() % 2 == 0)
	{
		self->action = &queenWaspAttackFinished;

		self->touch = &queenWaspRamTouch;
	}

	else
	{
		self->action = &queenWaspMoveToHeadButtRange;
	}
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
		showErrorAndExit("Fly boss is missing target");
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
	self->dirX = (self->face == LEFT ? 4 : -4);
	self->dirY = -6;

	setCustomAction(self, &helpless, 15, 0, 0);

	checkToMap(self);
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

		addDamageScore(damage, self);
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
		if (self->action == &queenWaspMoveToHeadButtRange || self->action == &queenWaspAttackFinished)
		{
			queenWaspReactToHeadButtBlock(other);
		}
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
	int x = (self->face == RIGHT ? 40 : 17);

	addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + 6, (self->face == RIGHT ? 7 : -7), -12);
	addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + 6, (self->face == RIGHT ? 4 : -4), -12);
	addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + 6, (self->face == RIGHT ? 1 : -1), -12);
	addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + 6, (self->face == RIGHT ? 2.5 : -2.5), -12);
	addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + 6, (self->face == RIGHT ? 5.5 : -5.5), -12);

	playSoundToMap("sound/boss/grub_boss/fire.ogg", BOSS_CHANNEL, self->x, self->y, 0);

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

		playSoundToMap("sound/common/crunch.ogg", -1, self->x, self->y, 0);

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

		playSoundToMap("sound/common/crunch.ogg", -1, self->x, self->y, 0);

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

		playSoundToMap("sound/common/crunch.ogg", -1, self->x, self->y, 0);

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

		playSoundToMap("sound/common/crunch.ogg", -1, self->x, self->y, 0);

		self->dirY = -14;
	}

	if (self->dirX == 0)
	{
		shakeScreen(MEDIUM, 15);

		playSoundToMap("sound/common/crunch.ogg", -1, self->x, self->y, 0);

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

			playSoundToMap("sound/boss/grub_boss/death.ogg", BOSS_CHANNEL, self->x, self->y, 0);
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

			self->thinkTime = 180;

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
			self->flags &= ~FLY;

			clearContinuePoint();

			increaseKillCount();

			freeBossHealthBar();

			e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

			e->dirY = ITEM_JUMP_HEIGHT;
		}

		else
		{
			blackBookShudder();
		}
	}
	
	checkToMap(self);
}

static void blackBookShudder()
{
	self->startY += 90;

	if (self->startY >= 360)
	{
		self->startY = 0;
	}

	self->x = self->startX + sin(DEG_TO_RAD(self->startY)) * 4;
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
		self->flags |= NO_DRAW;

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
