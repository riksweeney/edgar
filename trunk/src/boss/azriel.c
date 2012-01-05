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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "../headers.h"

#include "../audio/audio.h"
#include "../collisions.h"
#include "../enemy/enemies.h"
#include "../entity.h"
#include "../event/script.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../map.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../world/target.h"

extern Entity *self;

static void initialise(void);
static void doIntro(void);
static void attackFinished(void);
static void entityWait(void);
static void raiseDeadInit(void);
static void raiseDeadMoveToTopTarget(void);
static void raiseDead(void);
static void raiseDeadFinish(void);
static void phantasmalBoltInit(void);
static void phantasmalBoltMoveToTarget(void);
static void phantasmalBolt(void);
static void phantasmalBoltFinish(void);
static void phantasmalBoltMove(void);
static void phantasmalBoltReflect(void);
static void scytheThrowInit(void);
static void scytheThrowMoveToTarget(void);
static void scytheThrow(void);
static void scytheThrowWait(void);
static void scytheMove(void);
static void soulWait(void);

Entity *addAzriel(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Azriel");
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
	Entity *e;
	Target *t;
	
	e = getFreeEntity();
	
	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Edgar's Soul");
	}
	
	t = getTargetByName("EDGAR_SOUL_TARGET");
	
	if (t == NULL)
	{
		showErrorAndExit("Azirel cannot find target");
	}
	
	loadProperties("boss/edgar_soul", e);

	e->x = t->x;
	e->y = t->y;

	e->action = &soulWait;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;
	
	e->mental = 30 * 60;
	
	self->target = e;
	
	e->target = self;

	setEntityAnimation(e, "STAND");
	
	self->flags |= LIMIT_TO_SCREEN;

	playDefaultBossMusic();

	initBossHealthBar();

	self->takeDamage = &takeDamage;

	self->action = &attackFinished;

	checkToMap(self);
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		if (self->target->mental <= 0)
		{
			self->action = &phantasmalBoltInit;
		}

		else
		{
			self->action = &raiseDeadInit;

			self->action = &spikeAttackInit;

			self->action = &scytheThrowInit;
		}
	}

	checkToMap(self);
}

static void soulStealInit()
{
	self->flags |= NO_DRAW;

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	self->thinkTime = 30;

	self->action = &soulStealMoveToPlayer;
}

static void soulStealMoveToPlayer()
{
	int mid;
	Target *t;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		facePlayer();

		self->flags &= ~NO_DRAW;

		t = getTargetByName("AZRIEL_LEFT_TARGET");

		if (t == NULL)
		{
			showErrorAndExit("Azriel cannot find target");
		}

		mid = getMapStartX() + SCREEN_WIDTH / 2;

		self->x = player.x < mid ? player.x + player.w + 16 : player.x - self->w - 16;

		self->y = t->y;

		self->targetX = player.x;

		player.flags |= GROUNDED;

		addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

		self->action = &soulSteal;

		self->mental = self->health - 150;
	}
}

static void soulSteal()
{
	player.x = self->targetX;

	if (self->health <= self->mental)
	{
		self->action = &soulStealFinish;
	}
}

static void soulStealFinish()
{
	Target *t = getTargetByName("AZRIEL_TOP_TARGET");

	self->flags |= NO_DRAW;

	addParticleExplosion(self->x + self->w / 2, self->y + self->h / 2);

	self->thinkTime = 30;

	if (t == NULL)
	{
		showErrorAndExit("Azriel cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;
}

static void spikeAttackInit()
{
	Target *t = getTargetByName("AZRIEL_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Azriel cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= self->speed;
	self->dirY *= self->speed;

	self->action = &spikeAttackMoveToTopTarget;

	self->thinkTime = 30;

	checkToMap(self);
}

static void spikeAttackMoveToTopTarget()
{
	Entity *e;

	if (atTarget())
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			e = getFreeEntity();

			loadProperties("boss/azriel_light_beam", e);

			e->head = self;

			e->x = getMapStartX() + prand() % SCREEN_WIDTH;

			e->x -= e->w;

			e->flags |= NO_DRAW;

			e->action = &beamWait;
			e->draw = &drawLoopingAnimationToMap;
			e->touch = &entityTouch;

			e->face = RIGHT;

			e->type = ENEMY;

			e->thinkTime = 360;

			e->mental = 0;

			self->action = &spikeAttackTopWait;

			self->mental = 1;
		}
	}

	checkToMap(self);
}

static void beamWait()
{
	int x, startX;
	Entity *e;

	if (self->mental == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			/* Left side of beam */

			x = self->x;

			startX = getMapStartX();

			for (;x>=startX;)
			{
				e = getFreeEntity();

				loadProperties("boss/azriel_ground_spikes", e);

				e->head = self;

				e->x = x - e->w;

				e->action = &spikeRise;
				e->draw = &drawLoopingAnimationToMap;
				e->touch = &entityTouch;

				e->face = RIGHT;

				e->type = ENEMY;

				e->thinkTime = 120;

				x = e->x;
			}

			/* Right side of beam */

			x = self->x + self->w;

			startX = getMapStartX() + SCREEN_WIDTH;

			for (;x<startX;)
			{
				e = getFreeEntity();

				loadProperties("boss/azriel_ground_spikes", e);

				e->head = self;

				e->x = x;

				e->action = &spikeRise;
				e->draw = &drawLoopingAnimationToMap;
				e->touch = &entityTouch;

				e->face = RIGHT;

				e->type = ENEMY;

				e->thinkTime = 120;

				x = e->x + e->w;
			}

			self->mental = 1;
		}
	}

	else if (self->mental == 2)
	{
		self->head->mental = 0;

		self->inUse = FALSE;
	}
}

static void spikeAttackWait()
{
	if (self->mental == 0)
	{
		self->action = &attackFinished;
	}
}

static void scytheThrowInit()
{
	Target *t;

	if (prand() % 2 == 0)
	{
		t = getTargetByName("AZRIEL_LEFT_TARGET");

		self->face = RIGHT;
	}

	else
	{
		t = getTargetByName("AZRIEL_RIGHT_TARGET");

		self->face = LEFT;
	}

	if (t == NULL)
	{
		showErrorAndExit("Azriel cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= self->speed;
	self->dirY *= self->speed;

	self->action = &scytheThrowMoveToTarget;

	checkToMap(self);
}

static void scytheThrowMoveToTarget()
{
	if (atTarget())
	{
		self->thinkTime = 30;

		self->mental = 3;

		self->action = &scytheThrow;
	}

	checkToMap(self);
}

static void scytheThrow()
{
	int distance;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getFreeEntity();

		loadProperties("boss/azriel_scythe", e);

		e->head = self;

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - self->offsetX;
		}

		else
		{
			e->x = self->x + self->offsetX;
		}

		e->y = self->y + self->offsetY;

		e->startX = e->x;

		e->dirX = self->face == LEFT ? -e->speed : e->speed;

		e->action = &scytheMove;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &entityTouch;

		e->face = self->face;

		e->type = ENEMY;

		e->flags |= FLY;

		distance = SCREEN_WIDTH / self->mental;

		e->targetX = self->face == LEFT ? e->x - distance : e->x + distance;

		self->action = &scytheWait;

		self->thinkTime = 1;

		self->mental--;
	}

	checkToMap(self);
}

static void scytheWait()
{
	if (self->thinkTime <= 0)
	{
		self->action = self->mental <= 0 ? &attackFinished : &scytheThrow;
	}

	checkToMap(self);
}

static void scytheMove()
{
	if (self->face == LEFT)
	{
		if (self->dirX < 0 && self->x <= self->targetX)
		{
			self->dirX *= -1;
		}

		else if (self->dirX > 0 && self->x >= self->startX)
		{
			self->head->thinkTime = 0;

			self->inUse = FALSE;
		}
	}

	else
	{
		if (self->dirX > 0 && self->x >= self->targetX)
		{
			self->dirX *= -1;
		}

		else if (self->dirX < 0 && self->x <= self->startX)
		{
			self->head->thinkTime = 0;

			self->inUse = FALSE;
		}
	}

	checkToMap(self);
}

static void phantasmalBoltInit()
{
	Target *t;

	if (prand() % 2 == 0)
	{
		t = getTargetByName("AZRIEL_LEFT_TARGET");

		self->face = RIGHT;
	}

	else
	{
		t = getTargetByName("AZRIEL_RIGHT_TARGET");

		self->face = LEFT;
	}

	if (t == NULL)
	{
		showErrorAndExit("Azriel cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= self->speed;
	self->dirY *= self->speed;

	self->action = &phantasmalBoltMoveToTarget;

	checkToMap(self);
}

static void phantasmalBoltMoveToTarget()
{
	if (atTarget())
	{
		self->thinkTime = 120;

		self->action = &phantasmalBolt;
	}

	checkToMap(self);
}

static void phantasmalBolt()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "ATTACK");

		e = addProjectile("boss/phantasmal_bolt", self, self->x, self->y, self->face == LEFT ? -8 : 8, 0);

		playSoundToMap("sound/boss/snake_boss/snake_boss_shot.ogg", -1, self->x, self->y, 0);

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - e->w - self->offsetX;
		}

		else
		{
			e->x = self->x + self->offsetX;
		}

		e->y = self->y + self->offsetY;

		e->face = self->face;

		e->action = &phantasmalBoltMove;

		e->flags |= FLY;

		e->reactToBlock = &phantasmalBoltReflect;

		e->thinkTime = 1200;

		e->mental = 2;

		self->thinkTime = 60;

		self->action = &phantasmalBoltFinish;

		self->endY = 0;
	}

	checkToMap(self);
}

static void phantasmalBoltFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void phantasmalBoltMove()
{
	Entity *e;

	self->mental--;

	if (self->mental <= 0)
	{
		e = addBasicDecoration(self->x, self->y, "decoration/skull_trail");

		if (e != NULL)
		{
			e->x = self->face == LEFT ? self->x + self->w - e->w : self->x;

			e->y = self->y + self->h / 2 - e->h / 2;

			e->y += (prand() % 8) * (prand() % 2 == 0 ? 1 : -1);

			e->thinkTime = 15 + prand() % 15;

			e->dirY = (1 + prand() % 10) * (prand() % 2 == 0 ? 1 : -1);

			e->dirY /= 10;
		}

		self->mental = 2;
	}

	checkToMap(self);
}

static void phantasmalBoltReflect(Entity *other)
{
	if (other->element != PHANTASMAL)
	{
		self->inUse = FALSE;

		return;
	}

	if (other->mental <= 7)
	{
		self->damage = 1200;
	}

	else if (other->mental <= 15)
	{
		self->damage = 600;
	}

	else if (other->mental <= 30)
	{
		self->damage = 300;
	}

	else
	{
		self->damage = 0;
	}

	self->parent = other;

	self->dirX = -self->dirX;

	self->face = self->face == LEFT ? RIGHT : LEFT;
}

static void raiseDeadInit()
{
	Target *t = getTargetByName("AZRIEL_TOP_TARGET");

	if (t == NULL)
	{
		showErrorAndExit("Azriel cannot find target");
	}

	self->targetX = t->x;
	self->targetY = t->y;

	calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

	self->dirX *= self->speed;
	self->dirY *= self->speed;

	self->action = &raiseDeadMoveToTopTarget;

	checkToMap(self);
}

static void raiseDeadMoveToTopTarget()
{
	char c;
	int i, j;

	if (atTarget())
	{
		self->thinkTime = 30;

		self->action = &raiseDead;

		self->mental = 3 + prand() % 4;

		STRNCPY(self->description, "123456", sizeof(self->description));

		for (i=0;i<6;i++)
		{
			j = prand() % 6;

			c = self->description[i];

			self->description[i] = self->description[j];

			self->description[j] = c;
		}
	}

	checkToMap(self);
}

static void raiseDead()
{
	char targetName[MAX_VALUE_LENGTH];
	Target *t;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		snprintf(targetName, MAX_VALUE_LENGTH, "GRAVE_%d", self->description[self->mental]);

		t = getTargetByName(targetName);

		if (t == NULL)
		{
			showErrorAndExit("Azriel cannot find target");
		}

		e = addEnemy("enemy/zombie", t->x, t->y);

		e->head = self;

		self->mental--;

		if (self->mental <= 0)
		{
			self->thinkTime = 30;

			self->action = &raiseDeadFinish;
		}
	}

	checkToMap(self);
}

static void raiseDeadFinish()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = getEntityByName("enemy/zombie");

		if (e == NULL)
		{
			self->action = &attackFinished;
		}

		else
		{
			self->thinkTime = 30;
		}
	}

	checkToMap(self);
}

static void attackFinished()
{
	self->mental = 0;

	self->damage = 1;

	self->thinkTime = 30;

	self->action = &entityWait;

	checkToMap(self);
}

static void becomeTransparent()
{
	self->endX--;

	if (self->endX <= 0)
	{
		if (self->alpha > 128)
		{
			self->alpha--;

			self->endX = 3;
		}

		else
		{
			self->alpha = 128;

			self->endX = 0;
		}
	}
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (self->alpha != 255)
	{
		if (other->element == PHANTASMAL)
		{
			self->alpha = 255;

			self->endX = damage;

			entityTakeDamageNoFlinch(other, damage);
		}

		else
		{
			if (prand() % 10 == 0)
			{
				setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
			}

			damage = 0;

			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
		}
	}

	else
	{
		entityTakeDamageNoFlinch(other, damage);
	}
}

static void soulWait()
{
	self->mental--;
	
	if (self->mental <= 0)
	{
		self->mental = 0;
	}
}
