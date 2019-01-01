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
#include "../entity.h"
#include "../event/script.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../hud.h"
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
static void introPause(void);
static void attackFinished(void);
static void dropAttackInit(void);
static void dropAttack(void);
static void riseAttackInit(void);
static void riseAttack(void);
static void takeDamage(Entity *, int);
static void touch(Entity *);
static void fireballAttackInit(void);
static void fireballAttack(void);
static void fireballAttackFinish(void);
static void stunned(void);
static void addStunStar(void);
static void starWait(void);
static void lavaWait(void);
static void stunFinish(void);
static void riseAttackFinish(void);
static void flameAttackInit(void);
static void flameAttack(void);
static void addFlame(void);
static void flameWait(void);
static int flameDraw(void);
static void flameAttackFinish(void);
static void die(void);
static void dieWait(void);
static void dieFinish(void);
static void revive(void);
static void fireWait(void);
static void becomeAsh(void);
static void resurrectInit(void);
static void becomePhoenix(void);
static void resurrect(void);
static void resurrectFlyUp(void);
static void flyToLava(void);

Entity *addPhoenix(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Phoenix");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;
	e->touch = NULL;

	e->draw = &drawLoopingAnimationToMap;

	e->creditsAction = &bossMoveToMiddle;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void initialise()
{
	Target *t;

	if (self->active == TRUE)
	{
		if (cameraAtMinimum())
		{
			setEntityAnimation(self, "ATTACK_1");

			centerMapOnEntity(NULL);

			t = getTargetByName("PHOENIX_TARGET_TOP_RIGHT");

			if (t == NULL)
			{
				showErrorAndExit("Phoenix cannot find target");
			}

			self->dirY = -12;

			self->flags |= ATTACKING;

			self->endY = t->y;

			setContinuePoint(FALSE, self->name, NULL);

			self->action = &doIntro;

			self->touch = &entityTouch;

			self->mental = 10;

			self->endX = 10;
		}
	}
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		checkToMap(self);

		if (self->y <= self->endY)
		{
			self->y = self->endY;

			self->dirY = 0;

			self->thinkTime = 120;

			self->action = &introPause;

			setEntityAnimation(self, "STAND");
		}
	}
}

static void introPause()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		playDefaultBossMusic();

		initBossHealthBar();

		setEntityAnimation(self, "STAND");

		self->action = &attackFinished;

		self->touch = &touch;

		self->takeDamage = &takeDamage;
	}
}

static void entityWait()
{
	int r;
	Target *t;

	self->thinkTime--;

	facePlayer();

	if (self->thinkTime <= 0 && player.health > 0)
	{
		r = prand() % 3;

		switch (r)
		{
			case 0:
				setEntityAnimation(self, "WALK");

				self->maxThinkTime = 3;

				self->action = &dropAttackInit;
			break;

			case 1:
				t = getTargetByName(prand() % 2 == 0 ? "PHOENIX_TARGET_TOP_LEFT" : "PHOENIX_TARGET_TOP_RIGHT");

				facePlayer();

				if (t == NULL)
				{
					showErrorAndExit("Phoenix cannot find target");
				}

				setEntityAnimation(self, "WALK");

				self->action = &flameAttackInit;

				self->targetX = t->x;
				self->targetY = t->y;

				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

				self->dirX *= 4;
				self->dirY *= 4;
			break;

			default:
				t = getTargetByName(self->x < player.x ? "PHOENIX_TARGET_BOTTOM_LEFT" : "PHOENIX_TARGET_BOTTOM_RIGHT");

				facePlayer();

				if (t == NULL)
				{
					showErrorAndExit("Phoenix cannot find target");
				}

				self->maxThinkTime = 5 + prand() % 3;

				self->targetX = t->x;
				self->targetY = t->y;

				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

				self->dirX *= 4;
				self->dirY *= 4;

				self->action = &fireballAttackInit;
			break;
		}
	}

	checkToMap(self);
}

static void attackFinished()
{
	facePlayer();

	setEntityAnimation(self, "STAND");

	self->thinkTime = 60;

	self->startX = 0;

	self->maxThinkTime = 0;

	self->action = &entityWait;
}

static void dropAttackInit()
{
	int min, max;

	min = getCameraMinX();

	max = getCameraMaxX();

	self->targetX = player.x - self->w / 2 + player.w / 2;

	/* Position above the player */

	if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;

		self->action = &dropAttack;

		self->thinkTime = 30;
	}

	else
	{
		self->dirX = self->targetX < self->x ? -player.speed * 3 : player.speed * 3;
	}

	checkToMap(self);

	if (self->x <= min || self->x + self->w >= max)
	{
		self->dirX = 0;

		self->x = self->x <= min ? min : max - self->w - 1;

		self->action = &dropAttack;

		self->thinkTime = 30;
	}
}

static void dropAttack()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "ATTACK_2");

		self->dirY = 12;
	}

	checkToMap(self);

	if (self->y > self->startY)
	{
		self->y = self->startY;

		self->dirY = 0;

		self->action = &riseAttackInit;
	}
}

static void riseAttackInit()
{
	int min, max;

	min = getCameraMinX();

	max = getCameraMaxX();

	self->targetX = player.x - self->w / 2 + player.w / 2;

	/* Position below the player */

	if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;

		self->action = &riseAttack;

		self->thinkTime = 30;

		setEntityAnimation(self, "ATTACK_1");
	}

	else
	{
		self->dirX = self->targetX < self->x ? -player.speed * 3 : player.speed * 3;
	}

	checkToMap(self);

	if (self->x <= min || self->x + self->w >= max)
	{
		self->dirX = 0;

		self->x = self->x <= min ? min : max - self->w - 1;

		self->action = &riseAttack;

		self->thinkTime = 30;

		setEntityAnimation(self, "ATTACK_1");
	}
}

static void riseAttack()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirY = -12;
	}

	checkToMap(self);

	if (self->y < self->endY)
	{
		self->y = self->endY;

		self->dirY = 0;

		self->maxThinkTime--;

		setEntityAnimation(self, "WALK");

		if (player.health <= 0)
		{
			self->maxThinkTime = 0;
		}

		self->thinkTime = 60;

		self->action = self->maxThinkTime > 0 ? &dropAttackInit : &riseAttackFinish;
	}
}

static void riseAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}
}

static void fireballAttackInit()
{
	if (atTarget())
	{
		self->x = self->targetX;

		self->y = self->targetY;

		self->dirX = 0;

		self->dirY = 0;

		self->action = &fireballAttack;
	}

	checkToMap(self);
}

static void fireballAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addProjectile("enemy/fireball", self, 0, 0, (self->face == LEFT ? -12 : 12), 0);

		if (self->face == LEFT)
		{
			e->x = self->x - e->w;
		}

		else
		{
			e->x = self->x + self->w + e->w;
		}

		e->y = self->y + self->h - 40;

		e->flags |= FLY;

		playSoundToMap("sound/enemy/fireball/fireball", -1, self->x, self->y, 0);

		self->thinkTime = 45;

		self->maxThinkTime--;

		if (self->maxThinkTime <= 0)
		{
			self->thinkTime = 60;

			self->action = &fireballAttackFinish;
		}
	}
}

static void fireballAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime < 0)
	{
		self->dirY = -4;
	}

	checkToMap(self);

	if (self->y < self->endY)
	{
		self->y = self->endY;

		self->dirY = 0;

		setEntityAnimation(self, "STAND");

		self->action = &attackFinished;
	}
}

static void flameAttackInit()
{
	facePlayer();

	if (atTarget())
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, "ATTACK_3");

			addFlame();

			playSoundToMap("sound/enemy/fire_burner/flame", BOSS_CHANNEL, self->x, self->y, 0);

			self->thinkTime = 30;

			self->dirX = self->face == LEFT ? -4 : 4;

			self->action = &flameAttack;
		}
	}

	checkToMap(self);
}

static void flameAttack()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, "WALK");

			self->maxThinkTime = 0;

			self->targetX = getCameraMinX() + SCREEN_WIDTH / 2 - self->w / 2;

			self->targetY = self->y;

			calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

			self->dirX *= 4;
			self->dirY *= 4;

			self->action = &flameAttackFinish;
		}
	}
}

static void flameAttackFinish()
{
	checkToMap(self);

	if (atTarget())
	{
		self->action = &attackFinished;
	}
}

static void addFlame()
{
	Entity *e = getFreeEntity();
	Target *t;

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Phoenix Flame");
	}

	loadProperties("boss/phoenix_flame", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &flameWait;
	e->touch = &entityTouch;

	e->draw = &flameDraw;

	e->type = ENEMY;

	e->head = self;

	e->startY = e->y;

	t = getTargetByName("PHOENIX_FLAME_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Phoenix cannot find target");
	}

	e->endY = t->y;

	self->maxThinkTime = 1;

	setEntityAnimation(e, "STAND");
}

static void flameWait()
{
	self->x = self->head->x + self->head->w / 2 - self->w / 2;
	self->y = self->head->y + self->offsetY;

	self->box.h = self->endY - self->startY;

	if (self->head->maxThinkTime != 1)
	{
		self->inUse = FALSE;
	}
}

static int flameDraw()
{
	drawLoopingAnimationToMap();

	while (self->y < self->endY)
	{
		drawSpriteToMap();

		self->y += self->h;
	}

	self->y = self->startY;

	return TRUE;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->mental > 0)
	{
		if (other->element == FIRE)
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

			if (other->type == PROJECTILE)
			{
				temp = self;

				self = other;

				self->die();

				self = temp;
			}
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
		}

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		damage = 0;
	}

	else
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
			if (other->element == FIRE)
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

				if (other->type == PROJECTILE)
				{
					temp = self;

					self = other;

					self->die();

					self = temp;
				}

				self->thinkTime = 0;
			}

			else
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

					if (self->pain != NULL)
					{
						self->pain();
					}
				}

				else
				{
					self->takeDamage = NULL;

					self->thinkTime = 120;

					self->startX = self->x;

					self->damage = 0;

					self->endX = 0;

					self->action = &die;
				}
			}
		}
	}
}

static void touch(Entity *other)
{
	if (self->health > 0 && other->type == KEY_ITEM && strcmpignorecase(other->name, "item/ice_cube") == 0)
	{
		self->mental--;

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		enemyPain();

		other->inUse = FALSE;

		if (self->mental == 0)
		{
			self->flags &= ~(FLY|ATTACKING);

			self->startX = 0;

			self->action = &stunned;

			setEntityAnimation(self, "CUSTOM_1");

			self->y -= 32;
		}
	}

	else
	{
		entityTouch(other);
	}
}

static void stunned()
{
	int i;

	if (self->standingOn != NULL)
	{
		if (self->startX == 0)
		{
			self->startX = 1;

			addStunStar();

			self->thinkTime = 600;

			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}

			playSoundToMap("sound/common/crash", BOSS_CHANNEL, self->x, self->y, 0);
		}

		else
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->mental = 99;

				self->action = &stunFinish;
			}
		}
	}

	checkToMap(self);

	if (self->environment == LAVA)
	{
		self->thinkTime = 180;

		self->mental = 10;

		self->action = &lavaWait;
	}
}

static void lavaWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= (FLY|ATTACKING);

		setEntityAnimation(self, "ATTACK_1");

		self->action = &riseAttack;
	}

	checkToMap(self);
}

static void stunFinish()
{
	setEntityAnimation(self, "CUSTOM_2");

	self->animationCallback = &revive;
}

static void revive()
{
	self->action = &revive;

	self->dirY = -6;

	setEntityAnimation(self, "WALK");

	self->flags |= (FLY|ATTACKING);

	if (self->y <= self->endY)
	{
		self->mental = 10;

		self->y = self->endY;

		self->dirY = 0;

		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void addStunStar()
{
	int i;
	Entity *e;

	for (i=0;i<2;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add the Phoenix's Star");
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

		e->x = self->x + self->w / 2 - e->w / 2;

		e->y = self->y - e->h;
	}
}

static void starWait()
{
	if (self->head->mental > 0 || self->head->health <= 0)
	{
		self->inUse = FALSE;
	}

	self->x = self->head->x + self->head->w / 2 - self->w / 2;

	self->y = self->head->y - self->h - 8;
}

static void die()
{
	self->thinkTime--;

	self->x = self->startX + sin(DEG_TO_RAD(self->endX)) * 4;

	self->endX += 90;

	if (self->endX >= 360)
	{
		self->endX = 0;
	}

	if (self->thinkTime <= 0)
	{
		self->x = self->startX;

		self->thinkTime = 10;

		self->mental = 40;

		self->maxThinkTime = self->mental;

		self->action = &dieWait;
	}

	checkToMap(self);
}

static void dieWait()
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

		loadProperties("boss/phoenix_die_fire", e);

		setEntityAnimation(e, "STAND");

		e->x = self->x + prand() % self->w;
		e->y = self->y + self->h - e->h;

		e->action = &fireWait;

		e->touch = &entityTouch;

		e->draw = &drawLoopingAnimationToMap;

		e->type = ENEMY;

		e->thinkTime = 30;

		e->health = 0;

		e->maxHealth = 3 + prand() % 3;

		e->mental = 1;

		e->head = self;

		self->thinkTime = 5;

		self->mental--;

		if (self->mental <= 0)
		{
			self->thinkTime = 90;

			self->action = &becomeAsh;
		}
	}

	checkToMap(self);
}

static void becomeAsh()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "DIE");

		self->action = &dieFinish;
	}

	checkToMap(self);
}

static void dieFinish()
{
	Entity *e;

	if (self->maxThinkTime == 0)
	{
		self->maxThinkTime = -1;

		if (self->mental <= 0)
		{
			increaseKillCount();

			freeBossHealthBar();

			e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

			e->y -= e->h;

			e->dirY = ITEM_JUMP_HEIGHT;

			runScript("lava_boss_die");
		}
	}

	else if (self->maxThinkTime == -1)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			e = addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");

			if (e != NULL)
			{
				e->dirY = -(5 + prand() % 20);

				e->dirY /= 10;
			}

			self->thinkTime = 5 + prand() % 10;
		}
	}

	if (self->mental == -1)
	{
		self->flags |= FLY;

		self->thinkTime = 60;

		self->mental = 40;

		self->maxThinkTime = self->mental;

		self->action = &resurrectInit;
	}

	checkToMap(self);
}

static void resurrectInit()
{
	Entity *e;

	if (cameraAtMinimum())
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add the Fire");
			}

			loadProperties("boss/phoenix_die_fire", e);

			setEntityAnimation(e, "STAND");

			e->x = self->x + prand() % self->w;
			e->y = self->y + self->h - e->h;

			e->action = &fireWait;

			e->touch = &entityTouch;

			e->draw = &drawLoopingAnimationToMap;

			e->type = ENEMY;

			e->thinkTime = 30;

			e->health = 0;

			e->maxHealth = 3 + prand() % 3;

			e->mental = 1;

			e->head = self;

			self->thinkTime = 5;

			self->mental--;

			if (self->mental <= 0)
			{
				self->thinkTime = 90;

				self->action = &becomePhoenix;
			}
		}
	}
}

static void becomePhoenix()
{
	int x, w;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		x = self->x;
		w = self->w;

		setEntityAnimation(self, "CUSTOM_2");

		self->frameSpeed = 0;

		self->x = x + w / 2 - self->w / 2;

		self->action = &resurrect;
	}

	checkToMap(self);
}

static void resurrect()
{
	if (self->maxThinkTime == 0)
	{
		self->maxThinkTime = -1;

		self->frameSpeed = 1;

		self->animationCallback = &resurrectFlyUp;
	}
}

static void resurrectFlyUp()
{
	self->action = &resurrectFlyUp;

	setEntityAnimation(self, "WALK");

	if (self->y > self->endY)
	{
		self->y -= 4;
	}

	else
	{
		self->y = self->endY;

		self->thinkTime = 60;

		self->action = &flyToLava;

		self->damage = 100;

		self->flags |= ATTACKING;
	}
}

static void flyToLava()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "ATTACK_2");

		self->dirY = 12;
	}

	checkToMap(self);

	if (self->y > self->startY)
	{
		self->y = self->startY;

		self->dirY = 0;

		clearContinuePoint();

		fadeBossMusic();

		entityDieNoDrop();
	}
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

			self->thinkTime = 360;

			self->mental *= -1;
		}

		else if (self->health < 0)
		{
			self->head->maxThinkTime--;

			self->inUse = FALSE;

			self->health = 0;
		}

		else
		{
			self->thinkTime = 20;
		}

		setEntityAnimationByID(self, self->health);
	}

	checkToMap(self);
}
