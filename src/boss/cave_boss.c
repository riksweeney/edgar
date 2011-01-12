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
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../audio/music.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"
#include "../graphics/decoration.h"
#include "../game.h"
#include "../hud.h"
#include "../map.h"
#include "../projectile.h"
#include "../item/key_items.h"
#include "../player.h"
#include "../enemy/enemies.h"
#include "../graphics/gib.h"
#include "../system/error.h"
#include "../world/target.h"
#include "../geometry.h"
#include "../enemy/egg.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void die(void);
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
static void spitFireInit(void);
static void spitFireMoveToTarget(void);
static void spitFire(void);
static void spitFireFinish(void);
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
static void spitIceInit(void);
static void spitIceMoveToTarget(void);
static void spitIce(void);
static void spitIceFinish(void);
static void iceTouch(Entity *);
static void eggDropInit(void);
static void eggDropMoveToTop(void);
static void eggDropMove(void);
static void dropEgg(void);

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

	e->draw = &drawLoopingAnimationToMap;

	e->die = &die;

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

		self->takeDamage = &takeDamage;

		self->action = &attackFinished;

		checkToMap(self);

		self->startX = 0;

		self->endX = 1;

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
			self->startX = 1;
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
								self->action = &spitFireInit;
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

				default: /* Ice */
					self->action = &spitIceInit;
					self->action = &eggDropInit;
				break;
			}
		}
	}
}

static void spitFireInit()
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
		setEntityAnimation(self, "FIRE_ATTACK");

		self->action = &spitFire;

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

		self->action = &spitFireMoveToTarget;

		if (self->y > self->targetY)
		{
			setEntityAnimation(self, "FIRE_WALK_UP");
		}

		self->dirX = 0;
		self->dirY = -self->speed;
	}
}

static void spitFireMoveToTarget()
{
	Target *t;

	checkToMap(self);

	if (self->dirY < 0 && self->y <= self->targetY)
	{
		self->y = self->targetY;

		self->dirY = 0;

		self->face = self->targetX < self->x ? LEFT : RIGHT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		setEntityAnimation(self, "FIRE_WALK");
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

		setEntityAnimation(self, "FIRE_WALK_UP");
	}

	else if (self->dirY > 0 && self->y >= self->targetY)
	{
		self->dirY = 0;

		setEntityAnimation(self, "FIRE_ATTACK");

		self->action = &spitFire;

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

			playSoundToMap("sound/enemy/fireball/fireball.ogg", BOSS_CHANNEL, self->x, self->y, 0);

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

			self->action = &spitFireFinish;
		}

		else
		{
			self->thinkTime = 30;
		}
	}

	checkToMap(self);
}

static void spitFireFinish()
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
			playSoundToMap("sound/enemy/fireball/fireball.ogg", BOSS_CHANNEL, self->x, self->y, 0);

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

		setEntityAnimation(e, "DOWN");

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
		self->dirX = 0;

		self->mental = 30;

		self->thinkTime = 3;

		self->action = &ceilingBurn;

		self->endY = getMapStartX();

		setEntityAnimation(self, "FIRE_ATTACK_UP");

		playSoundToMap("sound/enemy/fireball/fireball.ogg", BOSS_CHANNEL, self->x, self->y, 0);
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

		setEntityAnimation(e, "UP");

		self->mental--;

		if (self->mental <= 0)
		{
			self->endY += TILE_SIZE / 2 - e->w / 2;

			self->thinkTime = 30;

			self->action = &ceilingBurnWait;
		}

		else
		{
			self->thinkTime = 3;
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

		setEntityAnimation(e, "DOWN");

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
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void changeToIceInit()
{
	int startX;
	Target *t;

	startX = getMapStartX() + SCREEN_WIDTH / 2;

	t = getTargetByName(self->x < startX ? "CAVE_BOSS_TARGET_LEFT" : "CAVE_BOSS_TARGET_RIGHT");

	self->startX = 1;

	self->endX = 2;

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
			setEntityAnimation(self, "WALK_UP");
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

	self->startX = 1;

	self->endX = 2;

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
			setEntityAnimation(self, "WALK_UP");
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

			setEntityAnimation(self, "WALK");
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

		setEntityAnimation(self, "WALK_UP");
	}

	else if (self->dirY > 0 && self->y >= self->targetY)
	{
		self->dirY = 0;

		setEntityAnimation(self, "STAND");

		self->action = self->resumeNormalFunction;

		facePlayer();
	}
}

static void spitIceInit()
{
	Target *t = getTargetByName(prand() % 2 == 0 ? "CAVE_BOSS_TARGET_LEFT" : "CAVE_BOSS_TARGET_RIGHT");

	if (t == NULL)
	{
		showErrorAndExit("Cave Boss cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	self->mental = 5;

	self->endY = 5;

	if (self->x == self->targetX && self->y == self->targetY)
	{
		setEntityAnimation(self, "ICE_ATTACK");

		self->action = &spitIce;

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

		self->action = &spitIceMoveToTarget;

		if (self->y > self->targetY)
		{
			setEntityAnimation(self, "ICE_WALK_UP");
		}

		self->dirX = 0;
		self->dirY = -self->speed;
	}
}

static void spitIceMoveToTarget()
{
	Target *t;

	checkToMap(self);

	if (self->dirY < 0 && self->y <= self->targetY)
	{
		self->y = self->targetY;

		self->dirY = 0;

		self->face = self->targetX < self->x ? LEFT : RIGHT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		setEntityAnimation(self, "ICE_WALK");
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

		setEntityAnimation(self, "ICE_WALK_UP");
	}

	else if (self->dirY > 0 && self->y >= self->targetY)
	{
		self->dirY = 0;

		setEntityAnimation(self, "ICE_ATTACK");

		self->action = &spitIce;

		facePlayer();

		self->mental = 3 + prand() % 3;

		self->thinkTime = 30;
	}
}

static void spitIce()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
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

			self->action = &spitIceFinish;
		}

		else
		{
			self->thinkTime = 30;
		}
	}

	checkToMap(self);
}

static void spitIceFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
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

		self->mental = 3 + prand() % 3;

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

		e->x = self->x + (self->w - e->w) / 2;
		e->y = self->y + (self->h - e->h) / 2;

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

static void changeToFire()
{
	if (self->alpha == 255)
	{
		playSoundToMap("sound/enemy/fireball/fireball.ogg", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &changeToFire;

		self->draw = &fireDraw;
	}

	self->alpha -= 3;

	if (self->alpha <= 0)
	{
		setEntityAnimation(self, "FIRE_STAND");

		self->alpha = 255;

		self->action = &attackFinished;

		self->draw = &drawLoopingAnimationToMap;

		self->mental = 0;

		self->startX = 1;

		self->endX = 1;

		/*self->maxThinkTime = 20;*/

		self->maxThinkTime = 1;
	}

	checkToMap(self);
}

static void changeToIce()
{
	if (self->alpha == 255)
	{
		playSoundToMap("sound/common/freeze.ogg", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &changeToIce;

		self->draw = &iceDraw;
	}

	self->alpha -= 3;

	if (self->alpha <= 0)
	{
		setEntityAnimation(self, "ICE_STAND");

		self->alpha = 255;

		self->action = &attackFinished;

		self->draw = &drawLoopingAnimationToMap;

		self->mental = 0;

		self->startX = 1;

		self->endX = 2;

		/*self->maxThinkTime = 20;*/

		self->maxThinkTime = 1;
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

		damage = 0;

		addDamageScore(damage, self);

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
	}

	else
	{
		entityTakeDamageNoFlinch(other, damage);
	}
}

static void die()
{
	Entity *e;

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		clearContinuePoint();

		increaseKillCount();

		freeBossHealthBar();

		e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

		e->dirY = ITEM_JUMP_HEIGHT;

		entityDieNoDrop();
	}
}

static void touch(Entity *other)
{
	if (self->startX == -1 && other->type == KEY_ITEM && strcmpignorecase(other->name, "item/stalactite") == 0)
	{
		self->takeDamage(other, 500);
	}

	else if (self->startX != -1 && self->endX == 1
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

	else if (self->startX != -1 && self->endX == 2
		&& other->type == PROJECTILE && strcmpignorecase(other->name, "weapon/flaming_arrow") == 0)
	{
		self->maxThinkTime--;

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		enemyPain();

		other->inUse = FALSE;

		if (self->maxThinkTime <= 0)
		{
			self->maxThinkTime = 150;

			self->startX = -1;
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

	setEntityAnimation(self, "STAND");

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

	setEntityAnimation(self, "STAND");

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

			playSoundToMap("sound/common/crash.ogg", BOSS_CHANNEL, self->x, self->y, 0);
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
			self->endX = 2;
		}

		else if (self->health <= 1000)
		{
			self->endX = 1;
		}

		else
		{
			self->endX = 0;
		}

		self->flags |= FLY;

		self->action = &attackFinished;
	}

	checkToMap(self);
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
