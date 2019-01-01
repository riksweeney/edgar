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
#include "../custom_actions.h"
#include "../entity.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../item/item.h"
#include "../map.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "enemies.h"
#include "rock.h"
#include "thunder_cloud.h"

extern Entity *self, player;
extern Game game;

static void walk(void);
static void entityWait(void);
static void changeWalkDirectionStart(void);
static void changeWalkDirection(void);
static void changeWalkDirectionFinish(void);
static void takeDamage(Entity *, int);
static void iceAttackStart(void);
static void iceAttack(void);
static void createIce(void);
static void iceAttackFinish(void);
static void iceBallMove(void);
static void iceFloorWait(void);
static void iceSpikeMove(void);
static void spikeTakeDamage(Entity *, int);
static void breatheFireInit(void);
static void breatheFireWait(void);
static void castLightningBolt(void);
static void lightningBolt(void);
static void becomeRampagingInit(void);
static void becomeRampagingStage1(void);
static void becomeRampagingStage2(void);
static void becomeRampagingFinish(void);
static void die(void);
static void creditsMove(void);

Entity *addMasterTortoise(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Master Tortoise");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &walk;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->takeDamage = &takeDamage;
	e->reactToBlock = &changeDirection;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void walk()
{
	moveLeftToRight();

	self->thinkTime--;

	if (self->health >= self->maxHealth + 200)
	{
		self->dirX = 0;

		self->action = &becomeRampagingInit;

		setCustomAction(self, &invulnerableNoFlash, 3600, 0, 0);
	}

	else
	{
		if (player.health > 0 && prand() % 60 == 0)
		{
			if (collision(self->x + (self->face == RIGHT ? self->w : -320), self->y, 320, self->h, player.x, player.y, player.w, player.h) == 1)
			{
				self->thinkTime = 15;

				self->action = prand() % 2 == 0 ? &castLightningBolt : &breatheFireInit;

				self->dirX = 0;
			}
		}

		else if (self->thinkTime <= 0)
		{
			self->dirX = 0;

			if (prand() % 3 == 0)
			{
				self->mental = -1;

				self->action = &changeWalkDirectionStart;
			}

			else
			{
				self->thinkTime = 60;

				self->action = &iceAttackStart;
			}
		}
	}
}

static void changeWalkDirectionStart()
{
	setEntityAnimation(self, "CUSTOM_1");

	self->action = &entityWait;

	self->animationCallback = &changeWalkDirection;

	self->thinkTime = 60;

	checkToMap(self);
}

static void changeWalkDirection()
{
	self->thinkTime--;

	self->action = &changeWalkDirection;

	setEntityAnimation(self, "CUSTOM_2");

	if (self->thinkTime <= 0)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->frameSpeed = -1;

		setEntityAnimation(self, "CUSTOM_1");

		self->animationCallback = &changeWalkDirectionFinish;

		self->action = &entityWait;
	}

	checkToMap(self);
}

static void changeWalkDirectionFinish()
{
	self->frameSpeed = 1;

	self->mental = 0;

	setEntityAnimation(self, "STAND");

	self->action = &walk;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	self->thinkTime = 120 + prand() % 120;

	checkToMap(self);
}

static void entityWait()
{
	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (damage != 0)
	{
		if (self->element == NO_ELEMENT)
		{
			if (other->element != NO_ELEMENT)
			{
				if (self->flags & INVULNERABLE)
				{
					return;
				}

				if (damage != 0)
				{
					self->health += damage;

					if (other->type == PROJECTILE)
					{
						other->target = self;
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

					if (prand() % 5 == 0)
					{
						setInfoBoxMessage(90, 255, 255, 255, _("The damage from this weapon is being absorbed..."));
					}
				}
			}

			else
			{
				entityTakeDamageNoFlinch(other, damage);

				if ((prand() % 3 == 0) && self->face == other->face && self->health > 0 && self->dirX != 0)
				{
					self->mental = -1;

					self->dirX = 0;

					self->action = &changeWalkDirectionStart;
				}
			}
		}

		else
		{
			if (other->type == WEAPON)
			{
				/* Damage the player instead */

				temp = self;

				self = other->parent;

				self->takeDamage(temp, temp->damage);

				self = temp;

				return;
			}

			else if (other->type == PROJECTILE)
			{
				self->health -= damage;

				other->target = self;
			}

			if (self->health > 0)
			{
				setCustomAction(self, &flashWhite, 6, 0, 0);
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

				if (self->pain != NULL)
				{
					self->pain();
				}
			}

			else
			{
				self->damage = 0;

				increaseKillCount();

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
}

static void castLightningBolt()
{
	Entity *e;

	self->thinkTime--;

	setEntityAnimation(self, "ATTACK_1");

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add lightning");
		}

		loadProperties("enemy/lightning", e);

		setEntityAnimation(e, "STAND");

		e->x = self->x + self->w / 2;
		e->y = self->y + self->h / 2;

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		e->targetX = player.x + player.w / 2 - e->w / 2;
		e->targetY = getMapCeiling(e->targetX, self->y);

		e->startY = e->targetY;
		e->endY   = getMapFloor(e->targetX, e->targetY);

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND);

		e->head = self;

		e->face = RIGHT;

		e->action = &lightningBolt;

		e->draw = &drawLoopingAnimationToMap;

		e->head = self;

		e->face = self->face;

		e->type = ENEMY;

		e->thinkTime = 0;

		e->flags |= FLY|DO_NOT_PERSIST;

		self->thinkTime = 30;

		self->action = &breatheFireWait;
	}
}

static void lightningBolt()
{
	int i, middle;
	Entity *e;

	self->flags |= NO_DRAW;

	self->thinkTime--;

	middle = -1;

	if (self->thinkTime <= 0)
	{
		playSoundToMap("sound/enemy/thunder_cloud/lightning", -1, self->targetX, self->startY, 0);

		for (i=self->startY;i<self->endY;i+=32)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add lightning");
			}

			loadProperties("enemy/lightning", e);

			setEntityAnimation(e, "STAND");

			if (i == self->startY)
			{
				middle = self->targetX + self->w / 2 - e->w / 2;
			}

			e->x = middle;
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

		self->inUse = FALSE;
	}
}

static void breatheFireInit()
{
	Entity *e;

	setEntityAnimation(self, "ATTACK_1");

	e = addProjectile("enemy/fireball", self, 0, 0, (self->face == LEFT ? -6 : 6), 0);

	if (self->face == LEFT)
	{
		e->x = self->x + self->w - e->w - self->offsetX;
	}

	else
	{
		e->x = self->x + self->offsetX;
	}

	e->y = self->y + self->offsetY;

	e->flags |= FLY;

	playSoundToMap("sound/enemy/fireball/fireball", -1, self->x, self->y, 0);

	self->thinkTime = 30;

	self->action = &breatheFireWait;

	checkToMap(self);
}

static void breatheFireWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime = 300 + prand() % 180;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->action = &walk;
	}

	checkToMap(self);
}

static void iceAttackStart()
{
	self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;

	self->frameSpeed = 0;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->frameSpeed = 1;

		setEntityAnimation(self, "CUSTOM_1");

		self->animationCallback = &createIce;
	}

	checkToMap(self);
}

static void createIce()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Ice Tortoise Ice Ball");
	}

	loadProperties("enemy/ice_tortoise_ice_ball", e);

	setEntityAnimation(e, "STAND");

	e->x = self->x + self->offsetX;
	e->y = self->y + self->offsetY;

	e->dirX = 2;
	e->dirY = -2;

	e->action = &iceBallMove;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->fallout = &entityDieNoDrop;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Ice Tortoise Ice Ball");
	}

	loadProperties("enemy/ice_tortoise_ice_ball", e);

	setEntityAnimation(e, "STAND");

	e->x = self->x + self->w - e->w - self->offsetX;
	e->y = self->y + self->offsetY;

	e->dirX = -2;
	e->dirY = -2;

	e->action = &iceBallMove;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->fallout = &entityDieNoDrop;

	self->frameSpeed = 1;

	setEntityAnimation(self, "CUSTOM_2");

	self->action = &iceAttack;

	self->thinkTime = 120;
}

static void iceAttack()
{
	self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->frameSpeed = -1;

		setEntityAnimation(self, "CUSTOM_1");

		self->animationCallback = &iceAttackFinish;

		self->action = &entityWait;
	}

	checkToMap(self);
}

static void iceAttackFinish()
{
	self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;

	setEntityAnimation(self, "STAND");

	self->frameSpeed = 1;

	self->action = &walk;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	self->thinkTime = 120 + prand() % 120;

	checkToMap(self);
}

static void iceBallMove()
{
	int x;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		x = mapTileAt(self->x / TILE_SIZE, (self->y + self->h + 5) / TILE_SIZE);

		if (x >= SOLID_TILE_START && x <= SOLID_TILE_END)
		{
			self->layer = MID_GROUND_LAYER;

			x = self->x + self->w / 2;

			setEntityAnimation(self, "WALK");

			self->x = x - self->w / 2;

			self->action = &iceFloorWait;

			self->y++;

			self->thinkTime = 30;
		}

		else
		{
			self->inUse = FALSE;
		}
	}

	else if (self->standingOn != NULL)
	{
		self->inUse = FALSE;
	}

	else
	{
		self->health--;

		if (self->health <= 0)
		{
			self->inUse = FALSE;
		}
	}
}

static void iceFloorWait()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add an Upside Down Ice Spike");
		}

		loadProperties("enemy/ice_spike_upside_down", e);

		setEntityAnimation(e, "STAND");

		e->x = self->x;
		e->y = self->y + self->h + 8;

		e->startY = self->y + self->h - e->h;

		e->action = &iceSpikeMove;
		e->touch = &entityTouch;
		e->takeDamage = &spikeTakeDamage;
		e->draw = &drawLoopingAnimationToMap;

		e->head = self;
	}
}

static void iceSpikeMove()
{
	int i;
	Entity *e;

	self->dirX = 0;

	if (self->y > self->startY)
	{
		self->y -= 4;

		if (self->y <= self->startY)
		{
			self->y = self->startY;

			if (self->head != NULL)
			{
				self->head->inUse = FALSE;

				self->head = NULL;
			}
		}
	}

	else
	{
		checkToMap(self);

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->takeDamage = NULL;

			playSoundToMap("sound/common/shatter", -1, self->x, self->y, 0);

			for (i=0;i<8;i++)
			{
				e = addTemporaryItem("misc/ice_spike_upside_down_piece", self->x, self->y, RIGHT, 0, 0);

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

			self->inUse = FALSE;

			if (self->head != NULL)
			{
				self->head->inUse = FALSE;
			}
		}
	}
}

static void spikeTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

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
			self->thinkTime = 0;

			self->takeDamage = NULL;
		}
	}
}

static void becomeRampagingInit()
{
	self->startY = self->y;

	setEntityAnimation(self, "CUSTOM_3");

	self->y = self->startY;

	self->dirY = 0;

	self->thinkTime = 60;

	checkToMap(self);

	self->action = &becomeRampagingStage1;

	self->creditsAction = &becomeRampagingStage1;

	if (game.status == IN_GAME)
	{
		fadeOutMusic(2000);
	}
}

static void becomeRampagingStage1()
{
	int i;
	long onGround = (self->flags & ON_GROUND);

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (onGround == 0)
		{
			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}
		}

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, "CUSTOM_4");

			self->thinkTime = 180;

			self->flags |= FLY;

			self->action = &becomeRampagingStage2;

			self->creditsAction = &becomeRampagingStage2;
		}
	}
}

static void becomeRampagingStage2()
{
	if (self->y > self->startY)
	{
		self->y--;
	}

	else
	{
		self->y = self->startY;

		self->thinkTime--;

		if (self->thinkTime % 30 == 0)
		{
			self->frameSpeed += 0.5;
		}

		else if (self->thinkTime <= 0)
		{
			self->thinkTime = 30;

			self->action = &becomeRampagingFinish;

			self->creditsAction = &becomeRampagingFinish;
		}
	}

	checkToMap(self);
}

static void becomeRampagingFinish()
{
	Entity *e;

	self->thinkTime--;

	checkToMap(self);

	if (self->thinkTime <= 0)
	{
		if (game.status == IN_GAME)
		{
			playDefaultBossMusic();
		}

		e = addEnemy("enemy/rampaging_master_tortoise", self->x, self->y);

		setEntityAnimation(e, "CUSTOM_2");

		e->face = self->face;

		e->startX = -1;

		e->flags |= SPAWNED_IN;

		e->spawnTime = self->spawnTime;

		self->inUse = FALSE;

		e->action = e->resumeNormalFunction;

		e->creditsAction = e->resumeNormalFunction;
	}
}

static void die()
{
	playSoundToMap("sound/enemy/tortoise/tortoise_die", -1, self->x, self->y, 0);

	entityDie();
}

static void creditsMove()
{
	self->mental++;

	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}

	if (self->mental != 0 && (self->mental % 400) == 0)
	{
		self->thinkTime = 60;

		self->dirX = 0;

		self->creditsAction = &becomeRampagingInit;
	}
}
