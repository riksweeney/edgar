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
#include "../enemy/rock.h"
#include "../entity.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/gib.h"
#include "../hud.h"
#include "../inventory.h"
#include "../item/bomb.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/explosion.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void attackFinished(void);
static void mouthWait(void);
static void addMouth(void);
static void takeDamage(Entity *, int);
static void tentacleWait(void);
static void addTentacles(void);
static void tentacleAttackInit(void);
static void tentacleAttack(void);
static void tentacleAttackFinish(void);
static void entityWait(void);
static int drawTentacle(void);
static void tentacleTakeDamage(Entity *, int);
static void tentacleAttackRetract(void);
static void fireRocks(void);
static void rockMove(void);
static void rockBlock(Entity *);
static void boulderMove(void);
static void bombMove(void);
static void fireRocksInit(void);
static void fireRocksFinish(void);
static void shudder(void);
static void addRedTentacles(void);
static void redTentacleTouch(Entity *);
static void redTentacleTakeDamage(Entity *, int);
static void redTentacleAppear(void);
static void redTentacleWait(void);
static void redTentacleDisappear(void);
static void redTentacleAttackWait(void);
static void redTentacleAttackInit(void);
static void redTentacleAttack(void);
static void redTentacleAttackFinish(void);
static void redTentacleExplode(void);
static void redTentacleExplodeFinish(void);
static void die(void);
static void die2(void);
static void slimePlayer(Entity *);
static void slimeDie(void);
static void dieFinish(void);
static void leave(void);
static void creditsMove(void);

Entity *addBorerBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Borer Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;
	e->touch = NULL;

	e->draw = &drawLoopingAnimationToMap;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void initialise()
{
	if (self->active == TRUE)
	{
		addMouth();

		addTentacles();

		self->mental = -1;

		self->action = &doIntro;

		setContinuePoint(FALSE, self->name, NULL);

		playSoundToMap("sound/boss/ant_lion/earthquake", BOSS_CHANNEL, self->x, self->y, -1);

		shakeScreen(LIGHT, -1);
	}
}

static void addTentacles()
{
	int i, h;
	Entity *e;

	h = TILE_SIZE + TILE_SIZE / 2;

	for (i=0;i<3;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Borer Boss Tentacle");
		}

		loadProperties("boss/borer_boss_tentacle", e);

		setEntityAnimation(e, "STAND");

		e->y = self->y + h - e->h / 2;

		e->mental = i + 1;

		e->startY = e->y - 3;
		e->endY = e->y + 3;

		e->face = RIGHT;

		e->touch = &entityTouch;

		e->die = &entityDieNoDrop;

		e->action = &tentacleWait;

		e->draw = &drawTentacle;

		e->creditsAction = &tentacleWait;

		e->type = ENEMY;

		e->head = self;

		e->thinkTime = 60 + prand() % 180;

		h += TILE_SIZE * 2;
	}

	self->targetX = 3;
}

static void addMouth()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Borer Boss Mouth");
	}

	loadProperties("boss/borer_boss_mouth", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &mouthWait;

	e->draw = &drawLoopingAnimationToMap;

	e->creditsAction = &mouthWait;

	e->type = ENEMY;

	e->head = self;

	setEntityAnimation(e, "STAND");
}

static void doIntro()
{
	self->x -=0.25;

	if (self->x <= self->startX)
	{
		self->x = self->startX;

		shakeScreen(LIGHT, 0);

		stopSound(BOSS_CHANNEL);

		self->action = &introPause;
	}
}

static void introPause()
{
	self->action = &attackFinished;

	playDefaultBossMusic();

	initBossHealthBar();

	self->takeDamage = &takeDamage;

	self->touch = &entityTouch;

	self->mental = 0;

	self->endX = getMapLeft(self->x, self->y);

	self->endY = getMapFloor(self->x, self->y);
}

static void attackFinished()
{
	self->thinkTime = 60;

	self->action = &entityWait;
}

static void entityWait()
{
	if (self->health <= 0)
	{
		self->mental = 150;

		self->action = &die;
	}

	else if (player.health <= 0)
	{
		self->targetX = self->x + self->w + 16;

		playSoundToMap("sound/boss/ant_lion/earthquake", BOSS_CHANNEL, self->x, self->y, -1);

		shakeScreen(LIGHT, -1);

		self->action = &leave;
	}

	else if (self->targetX <= 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->mental == 0)
			{
				self->action = &addRedTentacles;
			}

			else
			{
				self->thinkTime = 300;

				self->mental = 10;

				playSoundToMap("sound/boss/borer_boss/breathe_in", BOSS_CHANNEL, self->x, self->y, 0);

				self->action = &fireRocksInit;
			}
		}
	}
}

static void fireRocksInit()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 30;

		self->action = &fireRocks;

		self->x = self->startX;
	}

	else
	{
		shudder();
	}

	e = addSmoke(self->endX + (prand() % (SCREEN_WIDTH / 2)), self->y, "decoration/dust");

	if (e != NULL)
	{
		e->y += prand() % self->h;

		calculatePath(e->x, e->y, self->x + self->w / 2, self->y + self->h / 2, &e->dirX, &e->dirY);

		e->dirX *= 6;
		e->dirY *= 6;
	}

	setCustomAction(&player, &attract, 2, 0, (player.x < (self->x + self->w / 2) ? (player.speed - 0.5) : -(player.speed - 0.5)));
}

static void fireRocks()
{
	int rand, fired;
	Entity *e;

	self->thinkTime--;

	self->x--;

	if (self->x <= self->startX)
	{
		self->x = self->startX;
	}

	if (self->thinkTime <= 0)
	{
		fired = FALSE;

		while (fired == FALSE)
		{
			rand = prand() % 12;

			switch (rand)
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
					e = getFreeEntity();

					if (e == NULL)
					{
						showErrorAndExit("No free slots to add a Borer projectile");
					}

					loadProperties("common/large_rock", e);

					setEntityAnimation(e, "STAND");

					e->x = self->x + self->w / 2 - e->w / 2;
					e->y = self->y + (TILE_SIZE + TILE_SIZE / 2) + (TILE_SIZE * 2 * (prand() % 3)) - e->h / 2;

					e->dirX = -12;

					e->flags |= FLY;

					e->action = &rockMove;
					e->touch = &entityTouch;
					e->reactToBlock = &rockBlock;
					e->draw = &drawLoopingAnimationToMap;

					e->type = ENEMY;

					fired = TRUE;
				break;

				case 5:
				case 6:
				case 7:
				case 8:
					e = getFreeEntity();

					if (e == NULL)
					{
						showErrorAndExit("No free slots to add a Borer projectile");
					}

					loadProperties("enemy/small_boulder", e);

					setEntityAnimation(e, "STAND");

					e->x = self->x + self->w / 2 - e->w / 2;

					switch (prand() % 3)
					{
						case 0:
							e->y = self->y;
						break;

						case 1:
							e->y = self->y + self->h / 2 - e->h / 2;
						break;

						default:
							e->y = self->y + self->h - e->h;
						break;
					}

					e->dirX = -12;

					e->flags |= FLY|UNBLOCKABLE;

					e->action = &boulderMove;
					e->touch = &entityTouch;
					e->draw = &drawLoopingAnimationToMap;

					e->type = ENEMY;

					fired = TRUE;
				break;

				/* Don't fire a bomb if there's one in the inventory or one on the screen */

				default:
					if (getInventoryItemByObjectiveName("Bomb") == NULL && getEntityByName("item/bomb") == NULL)
					{
						e = addBomb(0, 0, "item/bomb");

						e->health = 1;

						e->x = self->x + self->w / 2 - e->w / 2;
						e->y = self->y + (TILE_SIZE + TILE_SIZE / 2) + (TILE_SIZE * 2 * (prand() % 3)) - e->h / 2;

						e->dirX = -12;

						e->flags |= FLY;

						e->thinkTime = 180;

						e->action = &bombMove;
						e->touch = NULL;

						fired = TRUE;
					}
				break;
			}
		}

		self->x += 8;

		self->mental--;

		self->thinkTime = 30;

		if (self->mental <= 0)
		{
			self->mental = 0;

			self->action = &fireRocksFinish;
		}
	}
}

static void fireRocksFinish()
{
	self->x--;

	if (self->x <= self->startX)
	{
		self->x = self->startX;

		self->action = &attackFinished;

		if (self->health == self->maxHealth && (prand() % 3 == 0))
		{
			setInfoBoxMessage(60, 255, 255, 255, _("Try getting the Borer to grab a bomb..."));
		}
	}
}

static void rockMove()
{
	Entity *e;

	checkToMap(self);

	if (self->dirX == 0)
	{
		playSoundToMap("sound/common/rock_bounce", -1, self->x, self->y, 0);

		e = addSmallRock(self->x, self->y, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		e->dirX = 3;
		e->dirY = -4;

		e = addSmallRock(self->x, self->y, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		e->dirX = 3;
		e->dirY = -8;

		self->inUse = FALSE;
	}
}

static void boulderMove()
{
	int i;
	Entity *e;

	checkToMap(self);

	if (self->dirX == 0)
	{
		for (i=0;i<4;i++)
		{
			e = addTemporaryItem("misc/small_boulder_piece", self->x, self->y, self->face, 0, 0);

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

			setEntityAnimationByID(e, i);

			e->thinkTime = 60 + (prand() % 60);
		}

		playSoundToMap("sound/common/rock_shatter", -1, self->x, self->y, 0);

		self->inUse = FALSE;
	}
}

static void bombMove()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		/*
		if (self->thinkTime > 0)
		{
			self->thinkTime--;

			if (self->thinkTime == 0)
			{
				self->resumeNormalFunction();
			}
		}
		*/
		self->dirX = 0;
		self->dirY = 0;

		self->touch = &keyItemTouch;
	}

	else if (self->dirX == 0)
	{
		self->flags &= ~FLY;

		self->dirX = 4;
		self->dirY = -6;
	}
}

static void rockBlock(Entity *other)
{
	self->dirX = 0;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (strcmpignorecase(other->name, "common/explosion") == 0)
	{
		setCustomAction(self, &flashWhite, 6, 0, 0);

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
	}

	else
	{
		playSoundToMap("sound/common/dink", -1, self->x, self->y, 0);

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
}

static void mouthWait()
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

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static void tentacleWait()
{
	self->x = self->head->x - 12;

	self->endX = self->head->x + self->head->w / 2;

	if (self->head->mental == 0 && self->head->x == self->head->startX)
	{
		self->takeDamage = &tentacleTakeDamage;

		if (self->thinkTime > 0)
		{
			self->thinkTime--;
		}

		else
		{
			self->thinkTime = 60;

			self->startX = self->x;

			self->dirY = 3.0f * (prand() % 2 == 0 ? -1 : 1);

			self->action = &tentacleAttackInit;
		}
	}

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}

static void tentacleAttackInit()
{
	self->y += self->dirY;

	if (self->y >= self->endY)
	{
		self->y = self->endY;

		self->dirY *= -1;
	}

	else if (self->y <= self->startY)
	{
		self->y = self->startY;

		self->dirY *= -1;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->y = self->startY + 8;

		self->dirX = -20;

		self->dirY = 0;

		self->action = &tentacleAttack;
	}
}

static void tentacleAttack()
{
	float dirX;

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0)
	{
		if (dirX != 0)
		{
			playSoundToMap("sound/common/crunch", BOSS_CHANNEL, self->x, self->y, 0);

			shakeScreen(MEDIUM, 15);
		}

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->dirX = 3;

			self->thinkTime = 45;

			self->action = &tentacleAttackRetract;
		}
	}

	self->box.w = self->startX - self->x;
}

static void tentacleAttackRetract()
{
	self->thinkTime--;

	self->x += self->dirX;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 30;

		self->dirX = 16;

		self->action = tentacleAttackFinish;
	}

	self->box.w = self->startX - self->x;
}

static void tentacleAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->x += self->dirX;

		if (self->x >= self->startX)
		{
			self->action = &tentacleWait;

			self->thinkTime = 60 + prand() % 180;
		}
	}

	self->box.w = self->startX - self->x;
}

static int drawTentacle()
{
	float startX;

	startX = self->x;

	/* Draw the tentacle first */

	self->x += self->w;

	setEntityAnimation(self, "WALK");

	while (self->x <= self->endX)
	{
		drawSpriteToMap();

		self->x += self->w;
	}

	/* Draw the tip */

	setEntityAnimation(self, "STAND");

	self->x = startX;

	drawLoopingAnimationToMap();

	return TRUE;
}

static void tentacleTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (damage != 0)
	{
		self->health -= damage;

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

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
			self->dirX = 0;

			self->head->targetX--;

			if (self->head->targetX <= 0)
			{
				self->head->mental = 1;
			}

			self->damage = 0;

			self->die();
		}
	}
}

static void shudder()
{
	self->x = self->startX + sin(DEG_TO_RAD(self->startY)) * 2;

	self->startY += 90;

	if (self->startY >= 360)
	{
		self->startY = 0;
	}
}

static void addRedTentacles()
{
	int i, h;
	Entity *e;

	h = TILE_SIZE + TILE_SIZE / 2;

	for (i=0;i<3;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Borer Boss Tentacle");
		}

		loadProperties("boss/borer_boss_tentacle_red", e);

		setEntityAnimation(e, "STAND");

		e->x = self->x + 128;
		e->y = self->y + h - e->h / 2;

		e->startX = self->x - 12;

		e->endX = e->x;

		e->mental = i + 1;

		e->startY = e->y - 3;
		e->endY = e->y + 3;

		e->face = RIGHT;

		e->touch = &redTentacleTouch;

		e->takeDamage = &redTentacleTakeDamage;

		e->action = &redTentacleAppear;

		e->draw = &drawTentacle;

		e->type = ENEMY;

		e->head = self;

		e->thinkTime = 60 + prand() % 180;

		h += TILE_SIZE * 2;
	}

	self->targetX = 3;

	self->thinkTime = 600;

	self->action = &redTentacleAttackWait;
}

static void redTentacleAttackWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental = -1;
	}

	if (self->targetX <= 0)
	{
		self->mental = 1;

		self->action = &attackFinished;
	}
}

static void redTentacleTouch(Entity *other)
{
	if (other->type == PLAYER && self->target == NULL)
	{
		self->target = other;

		self->maxThinkTime = self->target->y;

		other->flags |= FLY;

		self->dirX = 2;

		self->action = &redTentacleAttackFinish;

		self->head->thinkTime = 0;
	}

	/* Don't pick up unlit bombs */

	else if (strcmpignorecase(other->name, "item/bomb") == 0 && self->target == NULL && other->mental == 1)
	{
		self->target = other;

		self->maxThinkTime = self->target->y;

		other->flags |= FLY;

		other->frameSpeed = 0;

		self->dirX = 6;

		self->action = &redTentacleAttackFinish;

		self->head->thinkTime = 0;
	}

	else
	{
		entityTouch(other);
	}
}

static void redTentacleTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (damage != 0)
	{
		self->health -= damage;

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

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

		else if (self->target != NULL)
		{
			self->target->flags &= ~FLY;

			self->target = NULL;

			self->touch = NULL;

			self->dirX = 12;

			self->action = &redTentacleAttackFinish;
		}
	}
}

static void redTentacleAppear()
{
	self->x -= 8;

	if (self->x <= self->startX)
	{
		self->x = self->startX;

		self->thinkTime = 30 + prand() % 180;

		self->action = &redTentacleWait;
	}
}

static void redTentacleWait()
{
	if (self->head->mental == -1)
	{
		self->action = &redTentacleDisappear;
	}

	else
	{
		if (self->thinkTime > 0)
		{
			self->thinkTime--;
		}

		else
		{
			self->thinkTime = 60;

			self->startX = self->x;

			self->dirY = 3.0f * (prand() % 2 == 0 ? -1 : 1);

			self->action = &redTentacleAttackInit;

			self->health = self->maxHealth;
		}
	}
}

static void redTentacleAttackInit()
{
	self->y += self->dirY;

	if (self->y >= self->endY)
	{
		self->y = self->endY;

		self->dirY *= -1;
	}

	else if (self->y <= self->startY)
	{
		self->y = self->startY;

		self->dirY *= -1;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->y = self->startY + 8;

		self->dirX = -20;

		self->dirY = 0;

		self->action = &redTentacleAttack;

		playSoundToMap("sound/boss/armour_boss/tongue_start", BOSS_CHANNEL, self->x, self->y, 0);
	}
}

static void redTentacleAttack()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->dirX = 20;

		self->action = &redTentacleAttackFinish;
	}
}

static void redTentacleAttackFinish()
{
	Entity *temp;

	self->x += self->dirX;

	if (self->target != NULL)
	{
		self->target->x = self->x + self->w / 2 - self->target->w / 2;

		self->target->y = self->maxThinkTime;

		self->box.w = self->startX - self->x;

		if (self->x >= self->endX && strcmpignorecase(self->target->name, "item/bomb") == 0)
		{
			self->head->thinkTime = 0;

			self->x = self->endX;

			self->health = 30;

			stopSound(self->target->endX);

			self->thinkTime = 30;

			self->action = &redTentacleExplode;

			self->target->inUse = FALSE;
		}

		else if (self->x >= self->endX && self->target->type == PLAYER)
		{
			self->head->thinkTime = 0;

			self->x = self->endX;

			self->action = &redTentacleWait;

			temp = self;

			self = self->target;

			self->die();

			self->flags |= NO_DRAW;

			self = temp;

			self->target = NULL;

			self->touch = NULL;
		}
	}

	else if (self->x >= self->startX)
	{
		self->x = self->startX;

		self->thinkTime = 30 + prand() % 180;

		self->action = self->health <= 0 ? &redTentacleDisappear : &redTentacleWait;
	}
}

static void redTentacleDisappear()
{
	self->x += 8;

	if (self->x >= self->endX)
	{
		self->inUse = FALSE;

		self->head->targetX--;
	}
}

static void redTentacleExplode()
{
	int x, y;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		x = self->head->x + self->head->w / 2;
		y = self->head->y + self->head->h / 2;

		x += (prand() % 32) * (prand() % 2 == 0 ? 1 : -1);
		y += (prand() % 32) * (prand() % 2 == 0 ? 1 : -1);

		e = addExplosion(x, y);

		e->layer = FOREGROUND_LAYER;

		self->health--;

		self->thinkTime = 5;

		if (self->health == 0)
		{
			self->head->health -= 500;

			self->thinkTime = 60;

			self->action = &redTentacleExplodeFinish;
		}
	}

	e = self;

	self = self->head;

	shudder();

	self = e;
}

static void redTentacleExplodeFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->head->targetX--;

		self->head->x = self->head->startX;

		self->inUse = FALSE;
	}
}

static void die()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addExplosion(0, 0);

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		e->targetX = self->x - (prand() % (SCREEN_WIDTH / 2));
		e->targetY = self->y + self->h / 2 + (prand() % (self->h / 8));

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->dirX *= 4;
		e->dirY *= prand() % 2 == 0 ? -4 : 4;

		e->damage = 0;

		e->mental = 1;

		e->touch = NULL;

		self->mental--;

		if (self->mental <= 0)
		{
			self->mental = 150;

			self->action = &die2;
		}

		else
		{
			self->thinkTime = 2;
		}
	}

	shudder();
}

static void die2()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addProjectile("boss/borer_boss_slime", self, self->x + (self->face == RIGHT ? self->w : 0), self->y + self->h / 2, (self->face == RIGHT ? 7 : -7), 0);

		e->flags |= FLY;

		e->touch = &slimePlayer;

		e->die = &slimeDie;

		e->x = self->x + self->w / 2 - e->w / 2;
		e->y = self->y + self->h / 2 - e->h / 2;

		e->targetX = self->x - (prand() % (SCREEN_WIDTH / 2));
		e->targetY = self->y + self->h / 2 + (prand() % (self->h / 8));

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->dirX *= 12;
		e->dirY *= prand() % 2 == 0 ? -12 : 12;

		e->damage = 0;

		self->mental--;

		if (self->mental <= 0)
		{
			freeEntityList(throwGibs("boss/borer_boss_gibs", 15));

			self->inUse = TRUE;

			self->flags |= NO_DRAW;

			self->touch = NULL;

			self->action = &dieFinish;

			self->thinkTime = 120;
		}

		else
		{
			self->thinkTime = 2;
		}
	}

	shudder();
}

static void slimePlayer(Entity *other)
{
	if (other->type == PLAYER)
	{
		other->dirX = 0;

		setPlayerSlimed(120);

		self->die();
	}
}

static void slimeDie()
{
	playSoundToMap("sound/common/splat3", -1, self->x, self->y, 0);

	self->inUse = FALSE;
}

static void dieFinish()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		clearContinuePoint();

		increaseKillCount();

		freeBossHealthBar();

		e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y + self->h / 2);

		e->dirY = ITEM_JUMP_HEIGHT;

		fadeBossMusic();

		entityDieVanish();
	}
}

static void leave()
{
	self->x +=0.25;

	if (self->x > self->targetX)
	{
		self->inUse = FALSE;

		shakeScreen(LIGHT, 0);

		stopSound(BOSS_CHANNEL);
	}
}

static void creditsMove()
{
	int x = self->targetX;

	addMouth();

	addTentacles();

	self->creditsAction = &bossMoveToMiddle;

	self->targetX = x;
}
