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
#include "../enemy/egg.h"
#include "../entity.h"
#include "../event/global_trigger.h"
#include "../event/trigger.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../inventory.h"
#include "../item/item.h"
#include "../item/key_items.h"
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
static void attackFinished(void);
static void takeDamage(Entity *, int);
static void touch(Entity *);
static void entityWait(void);
static void changeToIceInit(void);
static void changeToFireInit(void);
static void changeToFire(void);
static void changeToIce(void);
static void incinerateInit(void);
static void incinerateMoveToTop(void);
static void incinerate(void);
static void incinerateWait(void);
static void fireWait(void);
static int fireDraw(void);
static int iceDraw(void);
static void fireDropInit(void);
static void fireDropMoveToTop(void);
static void fireDropMoveAbovePlayer(void);
static void fireDrop(void);
static void fireDropFinish(void);
static void fireFall(void);
static void ceilingBurnInit(void);
static void ceilingBurnMoveToTop(void);
static void ceilingBurn(void);
static void ceilingBurnWait(void);
static void stunned(void);
static void stunFinish(void);
static void moveToTarget(void);
static void starWait(void);
static void iceTouch(Entity *);
static void eggDropInit(void);
static void eggDropMoveToTop(void);
static void eggDropMove(void);
static void dropEgg(void);
static void icicleDropInit(void);
static void icicleDropMoveToTop(void);
static void icicleDrop(void);
static void icicleDropWait(void);
static void icicleDropFinish(void);
static void iceDrop(void);
static void stunWake(void);
static void groundChargeInit(void);
static void groundCharge(void);
static void groundChargeFinish(void);
static void die(void);
static void dieFinish(void);
static void acidStreamInit(void);
static void acidStreamMoveToTop(void);
static void acidStream(void);
static void acidStreamFinish(void);
static void spitInit(void);
static void spitMoveToTarget(void);
static void spitFinish(void);
static void spitIce(void);
static void spitFire(void);
static void spitAcid(void);
static void finalAttack(void);
static void slimePlayer(Entity *);
static void dieWait(void);
static void addElementParticles(void);
static int drawCaveBoss(void);

Entity *addCaveBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Cave Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawCaveBoss;

	e->die = &die;

	e->creditsAction = &bossMoveToMiddle;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "NORMAL_STAND");

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

			self->thinkTime = 60;

			self->touch = &touch;

			setContinuePoint(FALSE, self->name, NULL);
		}
	}

	checkToMap(self);
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= LIMIT_TO_SCREEN;

		playDefaultBossMusic();

		initBossHealthBar();

		self->action = &attackFinished;

		self->startX = 0;

		self->endX = 0;

		self->thinkTime = 0;
	}

	checkToMap(self);
}

static void entityWait()
{
	int action;

	if (self->startX == 0)
	{
		if (self->endX == 0)
		{
			self->action = &changeToFireInit;
		}

		else if (self->endX == 1)
		{
			self->action = &changeToIceInit;
		}

		else
		{
			self->startX = -1;
		}

		checkToMap(self);
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime <= 0 && player.health > 0)
		{
			switch ((int)self->endX)
			{
				case 1: /* Fire */
					/* If the player is above then burn the roof */

					if (player.y < self->y)
					{
						self->action = &ceilingBurnInit;
					}

					else
					{
						action = prand() % 7;

						switch (action)
						{
							case 0:
							case 1:
							case 2:
								self->action = &spitInit;
							break;

							case 3:
							case 4:
							case 5:
								self->action = &fireDropInit;
							break;

							default:
								self->action = &incinerateInit;
							break;
						}
					}
				break;

				case 2: /* Ice */
					/* If the player has no arrows then drop some eggs */

					if (getInventoryItemByName("weapon/normal_arrow") == NULL && getInventoryItemByName("weapon/flaming_arrow") == NULL
						&& getEntityByName("enemy/baby_salamander") == NULL)
					{
						self->action = &eggDropInit;
					}

					else
					{
						action = prand() % 7;

						switch (action)
						{
							case 0:
							case 1:
							case 2:
								self->action = &spitInit;
							break;

							case 3:
							case 4:
							case 5:
								self->action = &icicleDropInit;
							break;

							default:
								self->action = &eggDropInit;
							break;
						}
					}
				break;

				default: /* Standard */
					action = prand() % 3;

					switch (action)
					{
						case 0:
							self->action = &groundChargeInit;
						break;

						case 1:
							self->action = &spitInit;
						break;

						default:
							self->action = &acidStreamInit;
						break;
					}
				break;
			}
		}
	}
}

static void spitInit()
{
	Target *t = getTargetByName(prand() % 2 == 0 ? "CAVE_BOSS_TARGET_LEFT" : "CAVE_BOSS_TARGET_RIGHT");

	if (t == NULL)
	{
		showErrorAndExit("Cave Boss cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	self->mental = 5;

	self->endY = prand() % 2 == 0 ? 3 : 5;

	if (self->x == self->targetX && self->y == self->targetY)
	{
		switch ((int)self->endX)
		{
			case 1:
				setEntityAnimation(self, "FIRE_ATTACK");

				self->action = &spitFire;
			break;

			case 2:
				setEntityAnimation(self, "ICE_ATTACK");

				self->action = &spitIce;
			break;

			default:
				setEntityAnimation(self, "NORMAL_ATTACK");

				self->action = &spitAcid;
			break;
		}

		facePlayer();

		self->mental = 3 + prand() % 3;

		self->thinkTime = 30;
	}

	else
	{
		t = getTargetByName("CAVE_BOSS_TARGET_TOP");

		if (t == NULL)
		{
			showErrorAndExit("Cave Boss cannot find target");
		}

		self->targetY = t->y;

		self->action = &spitMoveToTarget;

		if (self->y > self->targetY)
		{
			switch ((int)self->endX)
			{
				case 1:
					setEntityAnimation(self, "FIRE_WALK_UP");
				break;

				case 2:
					setEntityAnimation(self, "ICE_WALK_UP");
				break;

				default:
					setEntityAnimation(self, "NORMAL_WALK_UP");
				break;
			}
		}

		self->dirX = 0;
		self->dirY = -self->speed;
	}
}

static void spitMoveToTarget()
{
	Target *t;

	checkToMap(self);

	if (self->dirY < 0 && self->y <= self->targetY)
	{
		self->y = self->targetY;

		self->dirY = 0;

		self->face = self->targetX < self->x ? LEFT : RIGHT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		switch ((int)self->endX)
		{
			case 1:
				setEntityAnimation(self, "FIRE_WALK");
			break;

			case 2:
				setEntityAnimation(self, "ICE_WALK");
			break;

			default:
				setEntityAnimation(self, "NORMAL_WALK");
			break;
		}
	}

	else if ((self->dirX < 0 && self->x <= self->targetX) || (self->dirX > 0 && self->x >= self->targetX))
	{
		t = getTargetByName(self->dirX < 0 ? "CAVE_BOSS_TARGET_LEFT" : "CAVE_BOSS_TARGET_RIGHT");

		if (t == NULL)
		{
			showErrorAndExit("Cave Boss cannot find target");
		}

		self->x = self->targetX;

		self->targetY = t->y;

		self->face = self->dirX < 0 ? RIGHT : LEFT;

		self->dirX = 0;

		self->dirY = self->speed;

		switch ((int)self->endX)
		{
			case 1:
				setEntityAnimation(self, "FIRE_WALK_UP");
			break;

			case 2:
				setEntityAnimation(self, "ICE_WALK_UP");
			break;

			default:
				setEntityAnimation(self, "NORMAL_WALK_UP");
			break;
		}
	}

	else if (self->dirY > 0 && self->y >= self->targetY)
	{
		self->dirY = 0;

		self->y = self->targetY;

		switch ((int)self->endX)
		{
			case 1:
				setEntityAnimation(self, "FIRE_ATTACK");

				self->action = &spitFire;
			break;

			case 2:
				setEntityAnimation(self, "ICE_ATTACK");

				self->action = &spitIce;
			break;

			default:
				setEntityAnimation(self, "NORMAL_ATTACK");

				self->action = &spitAcid;
			break;
		}

		facePlayer();

		self->mental = 3 + prand() % 3;

		self->thinkTime = 30;
	}
}

static void spitFire()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		for (i=0;i<self->endY;i++)
		{
			e = addProjectile("enemy/fireball", self, self->x, self->y, (self->face == RIGHT ? 2 : -2), 0);

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add a Fireball");
			}

			playSoundToMap("sound/enemy/fireball/fireball", BOSS_CHANNEL, self->x, self->y, 0);

			e->damage = 1;

			e->face = self->face;

			e->flags |= PLAYER_TOUCH_ONLY;

			if (self->face == LEFT)
			{
				e->x = self->x + self->w - e->w - self->offsetX;
			}

			else
			{
				e->x = self->x + self->offsetX;
			}

			e->y = self->y + self->offsetY;

			if (self->endY == 3)
			{
				if (i != 0)
				{
					e->dirY = i == 1 ? -0.5 : 0.5;
				}
			}

			else
			{
				if (i == 0)
				{
					e->dirY = 0;
				}

				else if (i < 3)
				{
					e->dirY = i == 1 ? -0.5 : 0.5;
				}

				else
				{
					e->dirY = i == 3 ? -1 : 1;
				}
			}

			e->dirX *= 3;
			e->dirY *= 3;
		}

		self->mental--;

		if (self->mental <= 0)
		{
			setEntityAnimation(self, "FIRE_STAND");

			self->thinkTime = 60;

			self->action = &spitFinish;
		}

		else
		{
			self->thinkTime = 30;
		}
	}

	checkToMap(self);
}

static void spitIce()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		facePlayer();

		e = addProjectile("enemy/ice", self, self->x, self->y, 0, 0);

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Ice");
		}

		e->face = self->face;

		e->flags |= PLAYER_TOUCH_ONLY|FLY;

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - e->w - self->offsetX;
		}

		else
		{
			e->x = self->x + self->offsetX;
		}

		e->y = self->y + self->offsetY;

		calculatePath(e->x, e->y, player.x + player.w / 2, player.y + player.h / 2, &e->dirX, &e->dirY);

		e->dirX *= 8;
		e->dirY *= 8;

		e->touch = &iceTouch;

		self->mental--;

		if (self->mental <= 0)
		{
			setEntityAnimation(self, "ICE_STAND");

			self->thinkTime = 60;

			self->action = &spitFinish;
		}

		else
		{
			self->thinkTime = 30;
		}
	}

	checkToMap(self);
}

static void spitAcid()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		for (i=0;i<3;i++)
		{
			e = addProjectile("boss/cave_boss_acid", self, self->x, self->y, (self->face == RIGHT ? 2 : -2), 0);

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add Acid");
			}

			e->damage = 1;

			e->face = self->face;

			e->flags |= PLAYER_TOUCH_ONLY|FLY;

			if (self->face == LEFT)
			{
				e->x = self->x + self->w - e->w - self->offsetX;
			}

			else
			{
				e->x = self->x + self->offsetX;
			}

			e->y = self->y + self->offsetY;

			if (i != 0)
			{
				e->dirY = i == 1 ? -0.5 : 0.5;
			}

			e->dirX *= 3;
			e->dirY *= 3;
		}

		self->mental--;

		if (self->mental <= 0)
		{
			setEntityAnimation(self, "NORMAL_STAND");

			self->thinkTime = 60;

			self->action = &spitFinish;
		}

		else
		{
			self->thinkTime = 30;
		}
	}

	checkToMap(self);
}

static void spitFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void fireDropInit()
{
	Target *t = getTargetByName("CAVE_BOSS_TARGET_TOP");

	if (t == NULL)
	{
		showErrorAndExit("Cave Boss cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	self->dirY = -self->speed;

	if (self->y > self->targetY)
	{
		setEntityAnimation(self, "FIRE_WALK_UP");
	}

	self->action = &fireDropMoveToTop;
}

static void fireDropMoveToTop()
{
	checkToMap(self);

	if (self->y <= self->targetY)
	{
		self->y = self->targetY;

		self->dirY = 0;

		self->startY = 3 + prand() % 3;

		self->action = &fireDropMoveAbovePlayer;

		setEntityAnimation(self, "FIRE_WALK");

		facePlayer();
	}
}

static void fireDropMoveAbovePlayer()
{
	if (self->face == RIGHT)
	{
		self->targetX = player.x - self->offsetX + player.w / 2;
	}

	else
	{
		self->targetX = player.x - (self->w - self->offsetX) + player.w / 2;
	}

	if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;

		self->thinkTime = 3;

		if (player.health > 0)
		{
			playSoundToMap("sound/enemy/fireball/fireball", BOSS_CHANNEL, self->x, self->y, 0);

			self->mental = 10;

			setEntityAnimation(self, "FIRE_ATTACK_DOWN");

			self->action = &fireDrop;
		}

		else
		{
			setEntityAnimation(self, "FIRE_ATTACK_DOWN");
		}
	}

	else
	{
		self->dirX = self->targetX < self->x ? -player.speed * 2 : player.speed * 2;

		setEntityAnimation(self, "FIRE_WALK");
	}

	checkToMap(self);
}

static void fireDrop()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Fire");
		}

		loadProperties("enemy/fire", e);

		setEntityAnimation(e, "DOWN");

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - self->offsetX;
		}

		else
		{
			e->x = self->x + self->offsetX;
		}

		e->y = self->y + self->offsetY;

		e->x -= e->w / 2;

		e->action = &fireFall;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;

		e->face = self->face;

		e->type = ENEMY;

		e->health = 0;

		self->mental--;

		if (self->mental <= 0)
		{
			self->thinkTime = 15;

			self->action = &fireDropFinish;
		}

		else
		{
			self->thinkTime = 3;
		}
	}
}

static void fireDropFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->startY--;

		if (self->startY <= 0)
		{
			self->action = &attackFinished;
		}

		else
		{
			facePlayer();

			self->action = &fireDropMoveAbovePlayer;
		}
	}

	checkToMap(self);
}

static void fireFall()
{
	if (!(self->flags & FLY) && (self->flags & ON_GROUND))
	{
		self->health--;

		if (self->health <= 0)
		{
			self->inUse = FALSE;
		}
	}

	else if ((self->flags & FLY) && self->dirY == 0)
	{
		self->inUse = FALSE;
	}

	checkToMap(self);
}

static void ceilingBurnInit()
{
	Target *t = getTargetByName("CAVE_BOSS_TARGET_TOP");

	if (t == NULL)
	{
		showErrorAndExit("Cave Boss cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	self->dirY = -self->speed;

	if (self->y > self->targetY)
	{
		setEntityAnimation(self, "FIRE_WALK_UP");
	}

	self->action = &ceilingBurnMoveToTop;
}

static void ceilingBurnMoveToTop()
{
	checkToMap(self);

	if (self->dirY < 0 && self->y <= self->targetY)
	{
		self->y = self->targetY;

		self->dirY = 0;

		setEntityAnimation(self, "FIRE_WALK");

		self->face = self->targetX < self->x ? LEFT : RIGHT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	else if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;

		self->mental = 30;

		self->thinkTime = 3;

		self->action = &ceilingBurn;

		self->endY = getMapStartX();

		setEntityAnimation(self, "FIRE_ATTACK_UP");

		playSoundToMap("sound/enemy/fireball/fireball", BOSS_CHANNEL, self->x, self->y, 0);
	}
}

static void ceilingBurn()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Fire");
		}

		loadProperties("enemy/fire", e);

		setEntityAnimation(e, "UP");

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - self->offsetX;
		}

		else
		{
			e->x = self->x + self->offsetX;
		}

		e->y = self->y + self->offsetY;

		e->x -= e->w / 2;

		e->action = &fireFall;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;

		e->face = self->face;

		e->type = ENEMY;

		e->flags |= FLY;

		e->thinkTime = 600;

		e->dirY = -15;

		self->mental--;

		if (self->mental <= 0)
		{
			self->endY += TILE_SIZE / 2 - e->w / 2;

			self->thinkTime = 30;

			self->action = &ceilingBurnWait;
		}

		else
		{
			self->thinkTime = 6;
		}
	}

	checkToMap(self);
}

static void ceilingBurnWait()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Fire");
		}

		loadProperties("enemy/fire", e);

		setEntityAnimation(e, "DOWN");

		e->x = self->endY;
		e->y = getMapStartY() - e->h;

		e->flags |= PLAYER_TOUCH_ONLY;

		e->action = &fireFall;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;

		e->face = self->face;

		e->type = ENEMY;

		e->health = 120;

		self->endY += TILE_SIZE;

		if (self->endY >= getMapStartX() + SCREEN_WIDTH)
		{
			self->action = &attackFinished;
		}

		else
		{
			self->thinkTime = 5;
		}
	}

	checkToMap(self);
}

static void fireWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->health += self->mental;

		if (self->health == self->maxHealth)
		{
			self->maxHealth = 5;

			self->thinkTime = 300;

			self->mental *= -1;
		}

		else if (self->health < 0)
		{
			self->head->endY--;

			self->inUse = FALSE;

			self->health = 0;
		}

		else
		{
			self->thinkTime = 5;
		}

		setEntityAnimationByID(self, self->health);
	}

	checkToMap(self);
}

static void incinerateInit()
{
	Target *t = getTargetByName("CAVE_BOSS_TARGET_TOP");

	if (t == NULL)
	{
		showErrorAndExit("Cave Boss cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	self->dirY = -self->speed;

	if (self->y > self->targetY)
	{
		setEntityAnimation(self, "FIRE_WALK_UP");
	}

	self->action = &incinerateMoveToTop;
}

static void incinerateMoveToTop()
{
	checkToMap(self);

	if (self->dirY < 0 && self->y <= self->targetY)
	{
		self->y = self->targetY;

		self->dirY = 0;

		setEntityAnimation(self, "FIRE_WALK");

		self->face = self->targetX < self->x ? LEFT : RIGHT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	else if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;

		self->mental = 0;

		self->thinkTime = 3;

		self->action = &incinerate;

		self->endY = 0;

		setEntityAnimation(self, "FIRE_ATTACK_DOWN");
	}
}

static void incinerate()
{
	int x, startX, startY;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Fire");
		}

		loadProperties("enemy/fire", e);

		setEntityAnimation(e, "DOWN");

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - self->offsetX;
		}

		else
		{
			e->x = self->x + self->offsetX;
		}

		e->y = self->y + self->offsetY;

		e->x -= e->w / 2;

		e->action = &fireFall;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;

		e->face = self->face;

		e->type = ENEMY;

		e->health = 0;

		self->mental++;

		if (self->mental == 60)
		{
			startX = getMapStartX();
			startY = getMapStartY() + SCREEN_HEIGHT - TILE_SIZE;

			x = 0;

			while (x < SCREEN_WIDTH)
			{
				e = getFreeEntity();

				if (e == NULL)
				{
					showErrorAndExit("No free slots to add the Fire");
				}

				loadProperties("boss/phoenix_die_fire", e);

				setEntityAnimation(e, "STAND");

				e->x = startX + x;
				e->y = startY - e->h;

				e->action = &fireWait;

				e->touch = &entityTouch;

				e->draw = &drawLoopingAnimationToMap;

				e->type = ENEMY;

				e->flags |= FLY;

				e->layer = FOREGROUND_LAYER;

				e->thinkTime = 30;

				e->damage = 1;

				e->health = 0;

				e->maxHealth = 5;

				e->mental = 1;

				e->head = self;

				x += e->w;

				self->endY++;
			}
		}

		else if (self->mental > 100)
		{
			self->thinkTime = 60;

			self->action = &incinerateWait;
		}

		else
		{
			self->thinkTime = 3;
		}
	}

	checkToMap(self);
}

static void incinerateWait()
{
	if (self->endY <= 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &attackFinished;
		}
	}

	checkToMap(self);
}

static void changeToIceInit()
{
	int startX;
	Target *t;

	startX = getMapStartX() + SCREEN_WIDTH / 2;

	t = getTargetByName(self->x < startX ? "CAVE_BOSS_TARGET_LEFT" : "CAVE_BOSS_TARGET_RIGHT");

	if (t == NULL)
	{
		showErrorAndExit("Cave Boss cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	if (self->x != self->targetX && self->y != self->targetY)
	{
		t = getTargetByName("CAVE_BOSS_TARGET_TOP");

		if (t == NULL)
		{
			showErrorAndExit("Cave Boss cannot find target");
		}

		self->targetY = t->y;

		if (self->y > self->targetY)
		{
			setEntityAnimation(self, "NORMAL_WALK_UP");
		}

		self->dirX = 0;
		self->dirY = -self->speed;

		self->action = &moveToTarget;

		self->resumeNormalFunction = &changeToIce;
	}

	else
	{
		self->action = &changeToIce;
	}
}

static void changeToFireInit()
{
	int startX;
	Target *t;

	startX = getMapStartX() + SCREEN_WIDTH / 2;

	t = getTargetByName(self->x < startX ? "CAVE_BOSS_TARGET_LEFT" : "CAVE_BOSS_TARGET_RIGHT");

	if (t == NULL)
	{
		showErrorAndExit("Cave Boss cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	if (self->x != self->targetX && self->y != self->targetY)
	{
		t = getTargetByName("CAVE_BOSS_TARGET_TOP");

		if (t == NULL)
		{
			showErrorAndExit("Cave Boss cannot find target");
		}

		self->targetY = t->y;

		if (self->y > self->targetY)
		{
			setEntityAnimation(self, "NORMAL_WALK_UP");
		}

		self->dirX = 0;
		self->dirY = -self->speed;

		self->action = &moveToTarget;

		self->resumeNormalFunction = &changeToFire;
	}

	else
	{
		self->action = &changeToFire;
	}
}

static void moveToTarget()
{
	Target *t;

	checkToMap(self);

	if (self->dirY < 0 && self->y <= self->targetY)
	{
		self->y = self->targetY;

		self->dirY = 0;

		self->face = self->targetX < self->x ? LEFT : RIGHT;

		if (self->x != self->targetX)
		{
			self->dirX = self->face == LEFT ? -self->speed : self->speed;

			setEntityAnimation(self, "NORMAL_WALK");
		}

		else
		{
			t = getTargetByName(self->dirX < 0 ? "CAVE_BOSS_TARGET_LEFT" : "CAVE_BOSS_TARGET_RIGHT");

			if (t == NULL)
			{
				showErrorAndExit("Cave Boss cannot find target");
			}

			self->x = self->targetX;

			self->targetY = t->y;

			self->face = self->dirX < 0 ? RIGHT : LEFT;

			self->dirX = 0;

			self->dirY = self->speed;

			setEntityAnimation(self, "NORMAL_WALK_UP");
		}
	}

	else if ((self->dirX < 0 && self->x <= self->targetX) || (self->dirX > 0 && self->x >= self->targetX))
	{
		t = getTargetByName(self->dirX < 0 ? "CAVE_BOSS_TARGET_LEFT" : "CAVE_BOSS_TARGET_RIGHT");

		if (t == NULL)
		{
			showErrorAndExit("Cave Boss cannot find target");
		}

		self->x = self->targetX;

		self->targetY = t->y;

		self->face = self->dirX < 0 ? RIGHT : LEFT;

		self->dirX = 0;

		self->dirY = self->speed;

		setEntityAnimation(self, "NORMAL_WALK_UP");
	}

	else if (self->dirY > 0 && self->y >= self->targetY)
	{
		self->dirY = 0;

		setEntityAnimation(self, "NORMAL_STAND");

		self->action = self->resumeNormalFunction;

		facePlayer();
	}
}

static void iceTouch(Entity *other)
{
	if (other->type == PLAYER && other->element != ICE && !(other->flags & INVULNERABLE) && other->health > 0)
	{
		setPlayerFrozen(120);

		self->inUse = FALSE;
	}
}

static void eggDropInit()
{
	Target *t = getTargetByName("CAVE_BOSS_TARGET_TOP");

	if (t == NULL)
	{
		showErrorAndExit("Cave Boss cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	self->dirY = -self->speed;

	if (self->y > self->targetY)
	{
		setEntityAnimation(self, "ICE_WALK_UP");
	}

	self->action = &eggDropMoveToTop;
}

static void eggDropMoveToTop()
{
	checkToMap(self);

	if (self->y <= self->targetY)
	{
		self->y = self->targetY;

		self->dirY = 0;

		self->mental = 1 + prand() % 2;

		self->action = &eggDropMove;

		setEntityAnimation(self, "ICE_WALK");

		facePlayer();

		self->targetX = getMapStartX();

		self->targetX += prand() % (SCREEN_WIDTH - self->w);
	}
}

static void eggDropMove()
{
	Entity *e;

	if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;

		self->action = &dropEgg;

		setEntityAnimation(self, "ICE_ATTACK_DOWN");

		e = addEgg(self->x, self->y, "boss/cave_boss_egg");

		e->flags |= LIMIT_TO_SCREEN;

		e->pain = &enemyPain;

		e->thinkTime = 120 + (prand() % 180);

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - self->offsetX;
		}

		else
		{
			e->x = self->x + self->offsetX;
		}

		e->y = self->y + self->offsetY;

		e->x -= e->w / 2;

		e->startX = e->x;
		e->startY = e->y;

		self->thinkTime = 30;
	}

	else
	{
		self->dirX = self->targetX < self->x ? -self->speed : self->speed;
	}

	checkToMap(self);
}

static void dropEgg()
{
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
			self->targetX = getMapStartX();

			self->targetX += prand() % (SCREEN_WIDTH - self->w);

			self->action = &eggDropMove;

			setEntityAnimation(self, "ICE_WALK");
		}
	}

	checkToMap(self);
}

static void icicleDropInit()
{
	Target *t = getTargetByName("CAVE_BOSS_TARGET_TOP");

	if (t == NULL)
	{
		showErrorAndExit("Cave Boss cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	self->dirY = -self->speed;

	if (self->y > self->targetY)
	{
		setEntityAnimation(self, "ICE_WALK_UP");
	}

	self->action = &icicleDropMoveToTop;
}

static void icicleDropMoveToTop()
{
	checkToMap(self);

	if (self->dirY < 0 && self->y <= self->targetY)
	{
		self->y = self->targetY;

		self->dirY = 0;

		setEntityAnimation(self, "ICE_WALK");

		self->face = self->targetX < self->x ? LEFT : RIGHT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	else if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;

		self->mental = 30;

		self->thinkTime = 3;

		self->action = &icicleDrop;

		self->endY = getMapStartX();

		setEntityAnimation(self, "ICE_ATTACK_UP");
	}
}

static void icicleDrop()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Ice");
		}

		loadProperties("enemy/ice", e);

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - self->offsetX;
		}

		else
		{
			e->x = self->x + self->offsetX;
		}

		e->y = self->y + self->offsetY;

		e->x -= e->w / 2;

		e->action = &fireFall;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;

		e->face = self->face;

		e->type = ENEMY;

		e->flags |= FLY;

		e->thinkTime = 600;

		e->dirY = -15;

		self->mental--;

		if (self->mental <= 0)
		{
			self->endY += TILE_SIZE / 2 - e->w / 2;

			self->thinkTime = 30;

			self->action = &icicleDropWait;
		}

		else
		{
			self->thinkTime = 6;
		}
	}

	checkToMap(self);
}

static void icicleDropWait()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Ice Spike");
		}

		loadProperties("enemy/ice_spike", e);

		setEntityAnimation(e, "STAND");

		e->x = self->endY;
		e->y = getMapStartY() - e->h;

		e->flags |= PLAYER_TOUCH_ONLY|FLY;

		e->action = &iceDrop;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;

		e->face = self->face;

		e->type = ENEMY;

		e->health = 120;

		e->thinkTime = 60;

		e->head = self;

		self->endY += e->w;

		self->mental++;

		if (self->endY >= getMapStartX() + SCREEN_WIDTH)
		{
			self->action = &icicleDropFinish;
		}

		else
		{
			self->thinkTime = 10;
		}
	}

	checkToMap(self);
}

static void icicleDropFinish()
{
	if (self->mental <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void iceDrop()
{
	int i;
	Entity *e;

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~FLY;
	}

	if (self->flags & ON_GROUND)
	{
		playSoundToMap("sound/common/shatter", -1, self->x, self->y, 0);

		for (i=0;i<8;i++)
		{
			e = addTemporaryItem("misc/ice_spike_piece", self->x, self->y, RIGHT, 0, 0);

			e->x = self->x + self->w / 2;
			e->x -= e->w / 2;

			e->y = self->y + self->h / 2;
			e->y -= e->h / 2;

			e->dirX = (prand() % 4) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = ITEM_JUMP_HEIGHT * 2 + (prand() % ITEM_JUMP_HEIGHT);

			setEntityAnimationByID(e, i);

			e->thinkTime = 60 + (prand() % 60);

			e->touch = NULL;
		}

		self->head->mental--;

		self->inUse = FALSE;
	}
}

static void groundChargeInit()
{
	checkToMap(self);

	if (self->flags & FLY)
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime = 120;

		self->flags &= ~FLY;

		facePlayer();
	}

	else if (self->flags & ON_GROUND)
	{
		self->mental = 3 + prand() % 3;

		setEntityAnimation(self, "WALK");

		self->action = &groundCharge;
	}
}

static void groundCharge()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->dirX = self->face == LEFT ? -self->speed : self->speed;

			self->dirX *= 3;
		}

		addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
	}

	else
	{
		if (self->dirX == 0)
		{
			self->mental--;

			self->face = self->face == LEFT ? RIGHT : LEFT;

			if (self->mental <= 0)
			{
				self->thinkTime = 60;

				setEntityAnimation(self, "STAND");

				self->action = &groundChargeFinish;
			}

			else
			{
				self->dirX = self->face == LEFT ? -self->speed : self->speed;

				self->dirX *= 3;
			}
		}

		addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
	}

	checkToMap(self);
}

static void groundChargeFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= FLY;

		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void acidStreamInit()
{
	Target *t = getTargetByName("CAVE_BOSS_TARGET_TOP");

	if (t == NULL)
	{
		showErrorAndExit("Cave Boss cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	self->dirY = -self->speed;

	if (self->y > self->targetY)
	{
		setEntityAnimation(self, "NORMAL_WALK_UP");
	}

	self->action = &acidStreamMoveToTop;
}

static void acidStreamMoveToTop()
{
	checkToMap(self);

	if (self->dirY < 0 && self->y <= self->targetY)
	{
		self->y = self->targetY;

		self->dirY = 0;

		setEntityAnimation(self, "NORMAL_WALK");

		self->face = prand() % 2 == 0 ? LEFT : RIGHT;

		self->targetX = self->face == LEFT ? getMapStartX() : getMapStartX() + SCREEN_WIDTH - self->w - 1;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	else if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;

		self->thinkTime = 120;

		self->endY = 0;

		self->action = &acidStream;

		setEntityAnimation(self, "NORMAL_ATTACK_DOWN");
	}
}

static void acidStream()
{
	Entity *e;

	self->endY--;

	if (self->endY <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Acid");
		}

		loadProperties("boss/cave_boss_acid", e);

		setEntityAnimation(e, "DOWN");

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - self->offsetX;
		}

		else
		{
			e->x = self->x + self->offsetX;
		}

		e->y = self->y + self->offsetY;

		e->x -= e->w / 2;

		e->action = &fireFall;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;

		e->face = self->face;

		e->type = ENEMY;

		e->health = 0;

		self->endY = 6;
	}

	if (self->mental == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, "NORMAL_ATTACK_DOWN_WALK");

			self->thinkTime = 60;

			self->mental = 1;

			self->dirX = self->face == LEFT ? self->speed : -self->speed;
		}
	}

	else
	{
		if (self->dirX == 0)
		{
			setEntityAnimation(self, "NORMAL_ATTACK_DOWN");

			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->targetX = getMapStartX() + SCREEN_WIDTH / 2 - self->w / 2;

				self->dirX = self->targetX < self->x ? -self->speed : self->speed;

				self->action = &acidStreamFinish;

				setEntityAnimation(self, "NORMAL_WALK");
			}
		}
	}

	checkToMap(self);
}

static void acidStreamFinish()
{
	if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;

		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void changeToFire()
{
	if (self->alpha == 255)
	{
		playSoundToMap("sound/enemy/fireball/fireball", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &changeToFire;

		self->draw = &fireDraw;
	}

	self->alpha -= 3;

	if (self->alpha <= 0)
	{
		setEntityAnimation(self, "FIRE_STAND");

		self->alpha = 255;

		self->action = &attackFinished;

		self->draw = &drawCaveBoss;

		self->takeDamage = &takeDamage;

		self->mental = 0;

		self->startX = 1;

		self->endX = 1;

		self->maxThinkTime = 15;
	}

	checkToMap(self);
}

static void changeToIce()
{
	if (self->alpha == 255)
	{
		playSoundToMap("sound/common/freeze", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &changeToIce;

		self->draw = &iceDraw;
	}

	self->alpha -= 3;

	if (self->alpha <= 0)
	{
		setEntityAnimation(self, "ICE_STAND");

		self->alpha = 255;

		self->action = &attackFinished;

		self->draw = &drawCaveBoss;

		self->takeDamage = &takeDamage;

		self->mental = 0;

		self->startX = 1;

		self->endX = 2;

		self->maxThinkTime = 20;
	}

	checkToMap(self);
}

static void attackFinished()
{
	self->mental = 0;

	self->damage = 1;

	self->thinkTime = 30;

	self->action = &entityWait;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (self->startX != -1)
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

		damage = 0;

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
	}

	else
	{
		entityTakeDamageNoFlinch(other, damage);
	}
}

static void die()
{
	int i;

	self->action = &die;

	setCustomAction(self, &invulnerableNoFlash, 240, 0, 0);

	setEntityAnimation(self, "STUNNED");

	self->flags &= ~FLY;

	self->dirX = 0;

	self->mental = self->x < getMapStartX() + SCREEN_WIDTH / 2 ? 0 : 1;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		for (i=0;i<20;i++)
		{
			addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
		}

		playSoundToMap("sound/common/crash", BOSS_CHANNEL, self->x, self->y, 0);

		self->thinkTime = 120;

		self->endY = 0;

		self->action = &dieFinish;
	}
}

static void dieFinish()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		setCustomAction(self, &invulnerableNoFlash, 240, 0, 0);

		if (self->thinkTime <= 0)
		{
			fireTrigger(self->objectiveName);

			fireGlobalTrigger(self->objectiveName);
		}
	}

	if (self->endY == 1)
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime = 60;

		facePlayer();

		self->action = &finalAttack;
	}

	checkToMap(self);
}

static void finalAttack()
{
	Entity *e;

	if (self->endY == 1)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, "GROUND_ATTACK");

			e = addProjectile("boss/fly_boss_slime", self, self->x + (self->face == RIGHT ? self->w : 0), self->y, (self->face == RIGHT ? 7 : -7), 0);

			e->touch = &slimePlayer;

			self->endY = 0;

			self->die = &dieWait;
		}
	}
}

static void slimePlayer(Entity *other)
{
	if (other->type == PLAYER)
	{
		other->dirX = 0;

		if (!(other->flags & HELPLESS))
		{
			setPlayerSlimed(30);
		}

		self->die();
	}
}

static void dieWait()
{
	Entity *e;

	clearContinuePoint();

	increaseKillCount();

	freeBossHealthBar();

	e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

	e->dirY = ITEM_JUMP_HEIGHT;

	fadeBossMusic();

	entityDieVanish();
}

static void touch(Entity *other)
{
	if (self->startX == -1 && other->type == KEY_ITEM && strcmpignorecase(other->name, "item/stalactite") == 0)
	{
		self->takeDamage(other, 500);

		other->mental = -2;
	}

	else if (self->startX == 1 && self->endX == 1
		&& other->type == KEY_ITEM && strcmpignorecase(other->name, "item/ice_cube") == 0)
	{
		self->maxThinkTime--;

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		enemyPain();

		other->inUse = FALSE;

		if (self->maxThinkTime <= 0)
		{
			self->startX = -1;

			setEntityAnimation(self, "STUNNED");

			self->flags &= ~FLY;

			self->damage = 0;

			self->dirX = 0;

			self->dirY = 0;

			self->thinkTime = 600;

			self->action = &stunned;
		}
	}

	else if (self->startX == 1 && self->endX == 2
		&& other->type == PROJECTILE && strcmpignorecase(other->name, "weapon/flaming_arrow") == 0)
	{
		self->maxThinkTime--;

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		enemyPain();

		other->inUse = FALSE;

		if (self->maxThinkTime <= 0)
		{
			self->startX = -1;

			setEntityAnimation(self, "STUNNED");

			self->flags &= ~FLY;

			self->damage = 0;

			self->dirX = 0;

			self->dirY = 0;

			self->thinkTime = 600;

			self->action = &stunned;
		}
	}

	else
	{
		entityTouch(other);
	}
}

static int fireDraw()
{
	int frame, alpha;
	float timer;

	/* Draw the boss with its lowering alpha */

	drawLoopingAnimationToMap();

	frame = self->currentFrame;
	timer = self->frameTimer;

	alpha = self->alpha;

	/* Draw the other part with its rising alpha */

	setEntityAnimation(self, "FIRE_STAND");

	self->currentFrame = frame;
	self->frameTimer = timer;

	self->alpha = 255 - alpha;

	drawSpriteToMap();

	/* Reset back to original */

	setEntityAnimation(self, "NORMAL_STAND");

	self->currentFrame = frame;
	self->frameTimer = timer;

	self->alpha = alpha;

	return 1;
}

static int iceDraw()
{
	int frame, alpha;
	float timer;

	/* Draw the boss with its lowering alpha */

	drawLoopingAnimationToMap();

	frame = self->currentFrame;
	timer = self->frameTimer;

	alpha = self->alpha;

	/* Draw the other part with its rising alpha */

	setEntityAnimation(self, "ICE_STAND");

	self->currentFrame = frame;
	self->frameTimer = timer;

	self->alpha = 255 - alpha;

	drawSpriteToMap();

	/* Reset back to original */

	setEntityAnimation(self, "NORMAL_STAND");

	self->currentFrame = frame;
	self->frameTimer = timer;

	self->alpha = alpha;

	return 1;
}

static void stunned()
{
	int i;
	long onGround = self->flags & ON_GROUND;
	Entity *e;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (onGround == 0)
		{
			for (i=0;i<2;i++)
			{
				e = getFreeEntity();

				if (e == NULL)
				{
					showErrorAndExit("No free slots to add the Cave Boss's Star");
				}

				loadProperties("boss/armour_boss_star", e);

				e->x = self->x;
				e->y = self->y;

				e->action = &starWait;

				e->draw = &drawLoopingAnimationToMap;

				e->thinkTime = self->thinkTime;

				e->head = self;

				setEntityAnimation(e, "STAND");

				e->currentFrame = (i == 0 ? 0 : 6);

				if (self->face == LEFT)
				{
					e->x = self->x + self->w - e->w - e->offsetX;
				}

				else
				{
					e->x = self->x + e->offsetX;
				}

				e->y = self->y + e->offsetY;
			}

			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}

			playSoundToMap("sound/common/crash", BOSS_CHANNEL, self->x, self->y, 0);
		}

		self->action = &stunFinish;
	}
}

static void stunFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->startX = 0;

		if (self->health <= 500)
		{
			self->endX = 3;
		}

		else if (self->health <= 1000)
		{
			self->takeDamage = NULL;

			self->endX = 1;
		}

		else
		{
			self->takeDamage = NULL;

			self->endX = 0;
		}

		setEntityAnimation(self, "STUN_WAKE");

		self->thinkTime = 60;

		self->action = &stunWake;
	}

	checkToMap(self);
}

static void stunWake()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= FLY;

		self->action = &attackFinished;
	}
}

static void starWait()
{
	self->face = self->head->face;

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->head->offsetX;
	}

	else
	{
		self->x = self->head->x + self->head->offsetX;
	}

	self->y = self->head->y + self->head->offsetY;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}

static void addElementParticles()
{
	Entity *e = NULL;

	if (self->startX == 1 && prand() % 3 == 0)
	{
		if (self->endX == 1)
		{
			e = addBasicDecoration(self->x, self->y, "decoration/small_flame");
		}

		else if (self->endX == 2)
		{
			e = addBasicDecoration(self->x, self->y, "decoration/small_ice");
		}

		if (e != NULL)
		{
			e->x = self->x + self->box.x;

			e->y = self->y + self->box.y;

			e->x += prand() % self->box.w;

			e->y += prand() % self->box.h;

			e->thinkTime = 30 + prand() % 30;

			e->dirY = -5 - prand() % 15;

			e->dirY /= 10;

			setEntityAnimationByID(e, 0);
		}
	}
}

static int drawCaveBoss()
{
	addElementParticles();

	return drawLoopingAnimationToMap();
}
