/*
Copyright (C) 2009 Parallel Realities

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
#include "../collisions.h"
#include "../custom_actions.h"
#include "../item/item.h"
#include "../projectile.h"
#include "../world/target.h"
#include "../geometry.h"
#include "../player.h"
#include "../enemy/enemies.h"
#include "../hud.h"
#include "../map.h"
#include "../event/script.h"
#include "../audio/music.h"
#include "../system/error.h"

extern Entity *self, player;

static void init(void);
static void firstEncounter(void);
static void firstEncounterMoveToPosition(void);
static void fireBouncingBalls(void);
static void fireBouncingBalls2(void);
static void hover(void);
static void firstEncounterDie(void);
static void shotBounce(void);
static void shotBounce2(void);
static void shotTouch(Entity *);
static void firstEncounterWait(void);
static void firstEncounterLeave(void);
static void shudder(void);
static void removeMaggot(void);
static void fireBouncingBallsFinish(void);
static void secondEncounter(void);
static void secondEncounterWait(void);
static void secondEncounterDie(void);
static void secondEncounterLeave(void);
static void bounceAway(void);

Entity *addFlyingMaggot(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Flying Maggot");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->action = &init;
	e->resumeNormalFunction = &init;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	if (self->active == TRUE)
	{
		switch (self->thinkTime)
		{
			case 0:
				self->flags &= ~NO_DRAW;
				self->action = &firstEncounter;
				self->die = &firstEncounterDie;
			break;

			case 1:
				self->action = &secondEncounter;
				self->die = &secondEncounterDie;
			break;
		}
	}
}

static void firstEncounter()
{
	Target *t;

	if (cameraAtMinimum())
	{
		centerMapOnEntity(NULL);

		self->takeDamage = &entityTakeDamageNoFlinch;

		t = getTargetByName("FIRST_ENCOUNTER_INTRO_TARGET");

		if (t == NULL)
		{
			showErrorAndExit("Flying Maggot cannot find target");
		}

		playSoundToMap("sound/boss/fly_boss/buzz.ogg", BOSS_CHANNEL, self->x, self->y, 0);

		self->targetX = t->x;
		self->targetY = t->y;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed;
		self->dirY *= self->speed;

		self->face = LEFT;

		self->action = &firstEncounterMoveToPosition;

		self->startX = 0;
	}
}

static void firstEncounterMoveToPosition()
{
	checkToMap(self);

	if (atTarget())
	{
		self->x = self->targetX;
		self->y = self->targetY;

		self->startY = self->y;

		self->dirY = 0;

		self->dirX = 0;

		setEntityAnimation(self, STAND);

		self->thinkTime = 300;

		self->action = &firstEncounterWait;

		playBossMusic();

		initBossHealthBar();

		self->touch = &entityTouch;
	}
}

static void firstEncounterWait()
{
	self->thinkTime--;

	self->action = &firstEncounterWait;

	self->frameSpeed = 1;

	if (self->thinkTime > 0)
	{
		setEntityAnimation(self, STAND);
	}

	else
	{
		self->maxThinkTime = 5;

		setEntityAnimation(self, ATTACK_1);

		self->animationCallback = &fireBouncingBalls;
	}

	hover();
}

static void fireBouncingBalls()
{
	Entity *e;

	self->thinkTime--;

	setEntityAnimation(self, ATTACK_2);

	self->action = &fireBouncingBalls;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		e = addProjectile("boss/flying_maggot_bounce_shot", self, self->x, self->y + 16, 0, 0);

		e->type = ENEMY;

		e->parent = NULL;

		e->face = self->face;

		e->dirX = self->face == LEFT ? -e->speed : e->speed;

		e->action = &shotBounce;

		e->touch = &shotTouch;

		e->die = &entityDieNoDrop;

		self->maxThinkTime--;

		if (self->maxThinkTime <= 0)
		{
			self->action = &fireBouncingBallsFinish;

			self->thinkTime = 300;
		}

		else
		{
			self->thinkTime = 60;
		}
	}

	hover();
}

static void secondEncounter()
{
	initBossHealthBar();

	self->thinkTime = 60;

	self->action = &secondEncounterWait;

	self->takeDamage = &entityTakeDamageNoFlinch;

	self->touch = &entityTouch;
}

static void secondEncounterWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime = 3;

		self->action = &fireBouncingBalls2;

		checkToMap(self);
	}
}

static void fireBouncingBalls2()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		e = addProjectile("boss/flying_maggot_bounce_shot_2", self, self->x + (self->face == RIGHT ? 113 : 3), self->y + 20, 0, 0);

		e->type = ENEMY;

		e->parent = NULL;

		e->face = self->face;

		e->dirX = self->face == LEFT ? -e->speed : e->speed;

		e->startY = prand() % 3 == 0 ? -11 : -10;

		e->action = &shotBounce2;

		e->touch = &shotTouch;

		e->reactToBlock = &bounceAway;

		e->takeDamage = &entityTakeDamageNoFlinch;

		e->die = &entityDieNoDrop;

		e->parent = self;

		self->maxThinkTime--;

		self->thinkTime = 120;

		if (self->maxThinkTime <= 0)
		{
			self->thinkTime = 300;

			self->action = &secondEncounterWait;
		}
	}
}

static void fireBouncingBallsFinish()
{
	self->frameSpeed = -1;

	setEntityAnimation(self, ATTACK_1);

	self->animationCallback = &firstEncounterWait;
}

static void firstEncounterDie()
{
	self->touch = NULL;

	self->thinkTime = 120;

	self->action = &firstEncounterLeave;

	self->startY = 0;

	self->startX = self->x;

	self->animationCallback = NULL;

	setEntityAnimation(self, STAND);
}

static void firstEncounterLeave()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->x = self->startX;

		self->die = &removeMaggot;

		runScript("flying_maggot_die_1");
	}

	shudder();
}

static void secondEncounterDie()
{
	self->thinkTime = 120;

	self->action = &secondEncounterLeave;

	self->dirX = 0;

	self->startY = 0;

	self->startX = self->x;

	self->animationCallback = NULL;

	setEntityAnimation(self, STAND);
}

static void secondEncounterLeave()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->x = self->startX;

		self->die = &removeMaggot;

		self->damage = 1;

		runScript("flying_maggot_die_2");
	}

	shudder();
}

static void shotBounce()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}

	if (self->flags & ON_GROUND)
	{
		self->dirY = -10;
	}
}

static void shotBounce2()
{
	checkToMap(self);

	if (self->parent->health <= 0)
	{
		self->die();
	}

	if (self->dirX == 0)
	{
		self->dirX = self->startX * -1;
	}

	if (self->flags & ON_GROUND)
	{
		self->dirY = self->startY;
	}

	self->startX = self->dirX;
}

static void shotTouch(Entity *other)
{
	int health;

	Entity *temp;

	if (other->type == PLAYER && self->damage != 0)
	{
		health = other->health;

		temp = self;

		self = other;

		self->takeDamage(temp, temp->damage);

		self = temp;

		if (other->health != health)
		{
			self->inUse = FALSE;
		}
	}

	else if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}
	}
}

static void bounceAway()
{
	self->dirX *= -1;
}

static void hover()
{
	self->startX++;

	if (self->startX >= 360)
	{
		self->startX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 16;
}

static void shudder()
{
	self->x = self->startX + sin(DEG_TO_RAD(self->startY)) * 4;

	self->startY += 90;

	if (self->startY >= 360)
	{
		self->startY = 0;
	}
}

static void removeMaggot()
{
	self->inUse = FALSE;

	freeBossHealthBar();

	fadeBossMusic();
}
