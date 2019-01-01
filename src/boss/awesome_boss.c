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
#include "../entity.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/graphics.h"
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
static void doIntro(void);
static void attackFinished(void);
static void entityWait(void);
static void introWait(void);
static void teleportIn(void);
static void teleportOut(void);
static void teleportWait(void);
static int energyBarDraw(void);
static void energyBarWait(void);
static void initEnergyBar(void);
static void takeDamage(Entity *, int);
static void die(void);
static void healPartner(void);
static void addStunStar(void);
static void starWait(void);
static void fireballAttackInit(void);
static void fireballAttack(void);
static void fireballAttackFinished(void);
static void superFireballAttackInit(void);
static void superFireballAttack(void);
static void superFireballAttackFinished(void);
static void fireballTouch(Entity *);
static void dropAttack(void);
static void dropAttackInit(void);
static void superDropAttack(void);
static void superDropAttackInit(void);
static void superDropAttackFinished(void);
static void fireballChargeWait(void);
static void superSpearAttackInit(void);
static void superSpearAttack(void);
static void superSpearAttackFinished(void);
static void addSmokeAlongBody(void);
static void spearWait(void);
static void spearRise(void);
static void spearSink(void);
static void fireballMove(void);
static void creditsMove(void);
static void creditsMoveToMiddle(void);

Entity *addAwesomeBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Awesome Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;

	e->takeDamage = &takeDamage;

	e->die = &die;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void initEnergyBar()
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

	e->head = self;

	setEntityAnimation(e, "STAND");

	self->head = getEntityByObjectiveName("AWESOME_BOSS_METER");

	if (self->head == NULL)
	{
		showErrorAndExit("Awesome Boss could not find meter");
	}

	self->head->damage++;
}

static void initialise()
{
	if (self->active == TRUE)
	{
		self->flags &= ~NO_DRAW;

		if (cameraAtMinimum())
		{
			centerMapOnEntity(NULL);

			self->thinkTime = 60;

			self->mental = 2;

			self->action = &doIntro;

			setContinuePoint(FALSE, self->name, NULL);
		}
	}

	checkToMap(self);
}

static void doIntro()
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

		e->face = RIGHT;

		e->active = TRUE;

		e->targetX = e->x;
		e->targetY = e->y;

		e->startX = e->x;
		e->startY = e->y;

		e->maxHealth = e->health = self->maxHealth;

		e->endX = self->mental;

		calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

		e->flags |= (NO_DRAW|HELPLESS|TELEPORTING|NO_END_TELEPORT_SOUND|LIMIT_TO_SCREEN);

		e->action = &introWait;

		e->head = self;

		playSoundToMap("sound/common/spell", -1, self->x, self->y, 0);

		self->thinkTime = 30;

		self->mental++;

		if (self->mental == 5)
		{
			self->target = getEntityByObjectiveName(self->requires);

			if (self->target == NULL)
			{
				showErrorAndExit("Awesome Boss %s cannot find %s", self->objectiveName, self->requires);
			}

			self->thinkTime = 60;

			self->action = &entityWait;
		}
	}
}

static void attackFinished()
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

		self->action = &teleportOut;
	}
}

static void teleportIn()
{
	if (player.health > 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &attackFinished;
		}
	}

	checkToMap(self);
}

static void teleportOut()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->touch = NULL;

		self->flags |= NO_DRAW;

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

		self->action = &teleportWait;

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

static void teleportWait()
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

			self->action = &teleportIn;

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

			self->action = &healPartner;

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

				self->action = superSpearAttackInit;
			}

			else
			{
				switch (self->head->targetX)
				{
					case 0:
						self->thinkTime = 30 * self->endX;

						self->mental = 5;

						self->action = &superDropAttackInit;

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

						self->action = superFireballAttackInit;
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
					self->action = &fireballAttackInit;
				break;

				default:
					self->action = &dropAttackInit;
				break;
			}

			setEntityAnimation(self, "STAND");

			self->dirY = 0;

			self->head->mental--;
		}
	}

	checkToMap(self);
}

static void fireballAttackInit()
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
	self->action = &fireballAttack;

	self->touch = &entityTouch;

	self->thinkTime = -1;
}

static void fireballAttack()
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

				e->action = &fireballChargeWait;
			}

			self->mental = 6;

			self->thinkTime = 0;
		}

		else if (self->mental <= 0)
		{
			setEntityAnimation(self, "ATTACK_2");

			e = addProjectile("boss/awesome_fireball", self, self->x, self->y, (self->face == RIGHT ? 14 : -14), 0);

			e->touch = &fireballTouch;

			e->action = &fireballMove;

			e->type = ENEMY;

			e->x += (self->face == RIGHT ? self->w : e->w);
			e->y += self->offsetY;

			self->thinkTime = 30;

			self->action = &fireballAttackFinished;

			playSoundToMap("sound/boss/awesome_boss/hadouken", -1, self->x, self->y, 0);
		}
	}
}

static void fireballAttackFinished()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime = 60;

		self->action = &attackFinished;
	}
}

static void superFireballAttackInit()
{
	facePlayer();

	playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

	self->flags &= ~NO_DRAW;
	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);
	self->action = &superFireballAttack;

	self->touch = &entityTouch;

	self->thinkTime = -1;

	self->head->mental = 0;
}

static void superFireballAttack()
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

				e->action = &fireballChargeWait;
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

					e->touch = &fireballTouch;

					e->action = &fireballMove;

					e->type = ENEMY;

					e->x += (self->face == RIGHT ? self->w : e->w);
					e->y += self->offsetY;

					e->x += self->face == RIGHT ? -2 : 2;
				}

				playSoundToMap("sound/boss/awesome_boss/hadouken", BOSS_CHANNEL, self->x, self->y, 0);

				self->thinkTime = 120;

				self->target->thinkTime = 120;

				self->action = &superFireballAttackFinished;

				self->target->action = &superFireballAttackFinished;
			}
		}
	}

	else if (self->endY == 0 && self->target->health <= 0)
	{
		self->thinkTime = 120;

		self->action = &teleportOut;
	}
}

static void superFireballAttackFinished()
{
	self->thinkTime--;

	self->head->health = 0;

	if (self->thinkTime <= 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void superSpearAttackInit()
{
	facePlayer();

	playSoundToMap("sound/common/spell", BOSS_CHANNEL, self->x, self->y, 0);

	self->flags &= ~NO_DRAW;
	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);
	self->action = &superSpearAttack;

	self->touch = &entityTouch;

	self->head->mental = 0;

	self->thinkTime = -1;

	if ((int)self->endX % 2 == 1)
	{
		self->endY = 1;
	}
}

static void superSpearAttack()
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

				e->action = &fireballChargeWait;
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

					e->action = &spearWait;

					e->draw = &drawLoopingAnimationToMap;

					e->head = self;

					i += e->w * 2;

					self->mental++;

					j++;
				}

				self->action = &superSpearAttackFinished;
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

					e->action = &spearWait;

					e->draw = &drawLoopingAnimationToMap;

					e->head = self;

					i -= e->w * 2;

					self->mental++;

					j++;
				}

				self->action = &superSpearAttackFinished;
			}
		}
	}
}

static void superSpearAttackFinished()
{
	self->head->health = 0;

	if (self->mental <= 0 || self->target->health <= 0)
	{
		self->action = &attackFinished;

		self->target->endY = 0;
	}

	checkToMap(self);
}

static void introWait()
{
	if (self->head->mental == 5 && self->head->thinkTime == 0)
	{
		self->target = getEntityByObjectiveName(self->requires);

		if (self->target == NULL)
		{
			showErrorAndExit("Awesome Boss %s cannot find %s", self->objectiveName, self->requires);
		}

		self->action = &teleportOut;

		self->touch = &entityTouch;

		initEnergyBar();
	}

	checkToMap(self);
}

static void entityWait()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			playDefaultBossMusic();

			initEnergyBar();

			self->head->active = TRUE;

			self->action = &teleportOut;

			self->touch = &entityTouch;
		}
	}

	checkToMap(self);
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

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		/* Take minimal damage from bombs */

		if (other->type == EXPLOSION)
		{
			damage = 1;
		}

		self->health -= damage;

		temp = self;

		self = self->head;

		self->takeDamage(other, damage);

		self = temp;

		if (self->health <= 0)
		{
			self->touch = NULL;

			setEntityAnimation(self, "DIE");

			addStunStar();

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

		if (self->pain != NULL)
		{
			self->pain();
		}
	}
}

static void die()
{
	if (self->target->health <= 0)
	{
		self->head->damage--;

		self->action = &entityDie;
	}

	checkToMap(self);
}

static void healPartner()
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

			self->target->action = &teleportOut;

			self->action = &teleportOut;

			setInfoBoxMessage(60, 255, 255, 255, _("Their partner will heal them when they fall..."));
		}
	}

	checkToMap(self);
}

static void dropAttackInit()
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

	self->action = &dropAttack;

	self->thinkTime = 60;

	self->dirY = 0;

	self->touch = &entityTouch;

	checkToMap(self);
}

static void dropAttack()
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
			self->action = &attackFinished;
		}
	}
}

static void superDropAttackInit()
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

		self->action = &superDropAttack;

		checkToMap(self);
	}
}

static void superDropAttack()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->mental--;

		if (self->mental <= 0)
		{
			addSmokeAlongBody();

			playSoundToMap("sound/enemy/red_grub/thud", -1, self->x, self->y, 0);

			self->action = &superDropAttackFinished;
		}

		else
		{
			addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

			self->flags |= (FLY|NO_DRAW);

			self->thinkTime = 90;

			self->x = player.x + player.w / 2 - self->w / 2;

			self->y = self->head->y;

			self->dirY = 0;

			self->action = &superDropAttackInit;

			self->head->mental = 0;
		}
	}
}

static void superDropAttackFinished()
{
	self->thinkTime = 60;

	self->head->health = 0;

	self->action = &attackFinished;

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
			showErrorAndExit("No free slots to add the Awesome Boss's Star");
		}

		loadProperties("boss/armour_boss_star", e);

		e->x = self->x;
		e->y = self->y;

		e->action = &starWait;

		e->draw = &drawLoopingAnimationToMap;

		e->thinkTime = 300;

		e->head = self;

		setEntityAnimation(e, "STAND");

		e->currentFrame = (i == 0 ? 0 : 6);

		e->x = self->x + self->w / 2 - e->w / 2;

		e->y = self->y - e->h;
	}
}

static void starWait()
{
	if (self->head->health == self->head->maxHealth || self->head->target->health <= 0)
	{
		self->inUse = FALSE;
	}

	self->x = self->head->x + self->head->w / 2 - self->w / 2;

	self->y = self->head->y - self->h - 8;
}

static void fireballTouch(Entity *other)
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

static void fireballChargeWait()
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

static void spearWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->y == self->startY)
		{
			self->action = &spearSink;
		}

		else
		{
			playSoundToMap("sound/enemy/ground_spear/spear", -1, self->x, self->y, 0);

			self->targetY = self->startY;

			self->action = &spearRise;
		}
	}
}

static void spearSink()
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

static void spearRise()
{
	if (self->y > self->startY)
	{
		self->y -= self->speed * 2;
	}

	else
	{
		self->y = self->startY;

		self->thinkTime = 5;

		self->action = &spearWait;
	}
}

static void fireballMove()
{
	self->thinkTime--;

	if (self->dirX == 0 || self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}

	checkToMap(self);
}

static void creditsMove()
{
	Entity *e;

	e = addEnemy("boss/awesome_boss_4", self->x + 24, self->y);

	e->endX = 24;

	e->head = self;

	e->face = LEFT;

	e->creditsAction = &creditsMoveToMiddle;

	e = addEnemy("boss/awesome_boss_3", self->x + 16, self->y);

	e->endX = 16;

	e->head = self;

	e->face = LEFT;

	e->creditsAction = &creditsMoveToMiddle;

	e = addEnemy("boss/awesome_boss_2", self->x + 8, self->y);

	e->endX = 8;

	e->head = self;

	e->face = LEFT;

	e->creditsAction = &creditsMoveToMiddle;

	e = addEnemy("boss/awesome_boss_1", self->x + 0, self->y);

	e->endX = 0;

	e->head = self;

	e->face = LEFT;

	e->creditsAction = &creditsMoveToMiddle;

	self->creditsAction = &bossMoveToMiddle;
}

static void creditsMoveToMiddle()
{
	self->x = self->head->x + self->endX;

	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}
