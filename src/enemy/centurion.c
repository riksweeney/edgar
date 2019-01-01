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
#include "../collisions.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../event/global_trigger.h"
#include "../event/trigger.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../item/item.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/explosion.h"

extern Entity *self, player;

static void lookForPlayer(void);
static void stompAttackInit(void);
static void stompAttack(void);
static void stompAttackFinish(void);
static void attacking(void);
static void die(void);
static void redDie(void);
static void reform(void);
static void dieWait(void);
static void pieceWait(void);
static void pieceReform(void);
static void pieceFallout(void);
static void reformFinish(void);
static void explosionAttackInit(void);
static void explosionAttack(void);
static void explosionAttackFinish(void);
static void explosionMove(void);
static void explode(void);
static void explosionTouch(Entity *);
static void redTakeDamage(Entity *, int);
static void creditsMove(void);
static void redCreditsMove(void);
static void creditsReformFinish(void);

Entity *addCenturion(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Centurion");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;
	e->die = strcmpignorecase(name, "enemy/red_centurion") == 0 ? &redDie : &die;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->takeDamage = strcmpignorecase(name, "enemy/red_centurion") == 0 ? &redTakeDamage : &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->creditsAction = strcmpignorecase(name, "enemy/red_centurion") == 0 ? &redCreditsMove : &creditsMove;;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void lookForPlayer()
{
	if (self->maxThinkTime == 1)
	{
		if (!(self->flags & ATTRACTED))
		{
			self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
		}
	}

	if (self->offsetX != 0)
	{
		if (self->maxThinkTime == 0)
		{
			if (self->flags & ON_GROUND)
			{
				playSoundToMap("sound/enemy/centurion/walk", -1, self->x, self->y, 0);
			}

			self->maxThinkTime = 1;
		}

		if (!(self->flags & ATTRACTED))
		{
			self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;
		}
	}

	else
	{
		self->maxThinkTime = 0;
	}

	if (self->maxThinkTime == 0 && (self->dirX == 0 || isAtEdge(self) == TRUE))
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}

	if (strcmpignorecase(self->name, "enemy/red_centurion") == 0)
	{
		if (self->thinkTime == 0 && !(player.flags & TELEPORTING) && player.health > 0 && prand() % 10 == 0)
		{
			if (collision(self->x + (self->face == RIGHT ? self->w : -240), self->y, 240, self->h, player.x, player.y, player.w, player.h) == 1)
			{
				self->action = &explosionAttackInit;

				if (!(self->flags & ATTRACTED))
				{
					self->dirX = 0;
				}
			}
		}
	}

	else
	{
		if (self->thinkTime == 0 && !(player.flags & TELEPORTING) && player.health > 0 && prand() % 15 == 0)
		{
			if (collision(self->x + (self->face == RIGHT ? self->w : -160), self->y, 160, self->h, player.x, player.y, player.w, player.h) == 1)
			{
				self->action = &stompAttackInit;

				if (!(self->flags & ATTRACTED))
				{
					self->dirX = 0;
				}
			}
		}
	}
}

static void stompAttackInit()
{
	setEntityAnimation(self, "ATTACK_1");

	self->animationCallback = &stompAttack;

	self->action = &attacking;

	checkToMap(self);
}

static void stompAttack()
{
	setEntityAnimation(self, "ATTACK_2");

	playSoundToMap("sound/common/crash", -1, self->x, self->y, 0);

	shakeScreen(MEDIUM, 60);

	self->thinkTime = 60;

	if (player.flags & ON_GROUND)
	{
		setPlayerStunned(120);
	}

	self->action = &stompAttackFinish;
}

static void stompAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		self->action = &lookForPlayer;

		self->thinkTime = 60;

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}
}

static void explosionAttackInit()
{
	setEntityAnimation(self, "ATTACK_1");

	self->animationCallback = &explosionAttack;

	self->action = &attacking;

	checkToMap(self);
}

static void explosionAttack()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a moving explosion");
	}

	loadProperties("common/explosion", e);

	setEntityAnimation(e, "STAND");

	e->x = self->x + self->w / 2 - e->w / 2;
	e->y = self->y + self->h - e->h;

	e->face = self->face;

	e->dirX = e->face == LEFT ? -6 : 6;

	e->damage = 1;

	e->action = &explosionMove;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &explosionTouch;

	e->type = ENEMY;

	e->flags |= NO_DRAW|DO_NOT_PERSIST;

	e->startX = playSoundToMap("sound/boss/ant_lion/earthquake", -1, self->x, self->y, -1);

	setEntityAnimation(self, "ATTACK_2");

	self->thinkTime = 60;

	self->action = &explosionAttackFinish;

	checkToMap(self);
}

static void explosionAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		self->action = &lookForPlayer;

		self->thinkTime = 60;

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}

	checkToMap(self);
}

static void explosionMove()
{
	Entity *e;

	e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

	if (e != NULL)
	{
		e->y -= prand() % e->h;
	}

	checkToMap(self);

	if (isAtEdge(self) == TRUE || self->dirX == 0)
	{
		self->touch = NULL;

		self->dirX = 0;

		self->mental = 5;

		self->action = &explode;
	}
}

static void explosionTouch(Entity *other)
{
	entityTouch(other);

	if (other->type == PLAYER)
	{
		self->touch = NULL;

		self->dirX = 0;

		self->mental = 5;

		self->action = &explode;
	}
}

static void explode()
{
	int x, y;
	Entity *e;

	self->thinkTime--;

	stopSound(self->startX);

	self->startX = -1;

	if (self->thinkTime <= 0)
	{
		x = self->x + self->w / 2;
		y = self->y + self->h / 2;

		x += (prand() % 32) * (prand() % 2 == 0 ? 1 : -1);
		y += (prand() % 32) * (prand() % 2 == 0 ? 1 : -1);

		e = addExplosion(x, y);

		e->type = ENEMY;

		e->damage = 1;

		self->mental--;

		self->thinkTime = 10;

		if (self->mental == 0)
		{
			self->inUse = FALSE;
		}
	}
}

static void attacking()
{
	checkToMap(self);
}

static void die()
{
	int i;
	Entity *e;
	char name[MAX_VALUE_LENGTH];

	snprintf(name, sizeof(name), "%s_piece", self->name);

	fireTrigger(self->objectiveName);

	fireGlobalTrigger(self->objectiveName);

	playSoundToMap("sound/enemy/centurion/centurion_die", -1, self->x, self->y, 0);

	for (i=0;i<9;i++)
	{
		e = addTemporaryItem(name, self->x, self->y, self->face, 0, 0);

		e->x += (self->w - e->w) / 2;
		e->y += (self->w - e->w) / 2;

		e->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->thinkTime = 60 + (prand() % 180);
	}

	self->damage = 0;

	if (!(self->flags & INVULNERABLE))
	{
		self->flags &= ~FLY;

		self->flags |= (DO_NOT_PERSIST|NO_DRAW);

		self->thinkTime = 60;

		setCustomAction(self, &invulnerableNoFlash, 240, 0, 0);

		self->frameSpeed = 0;

		self->action = &standardDie;

		self->damage = 0;
	}
}

static void redDie()
{
	int i;
	Entity *e;
	char name[MAX_VALUE_LENGTH];

	playSoundToMap("sound/enemy/centurion/centurion_die", -1, self->x, self->y, 0);

	snprintf(name, sizeof(name), "%s_piece", self->name);

	for (i=0;i<9;i++)
	{
		e = addTemporaryItem(name, self->x, self->y, self->face, 0, 0);

		e->action = &pieceWait;

		e->creditsAction = &pieceWait;

		e->fallout = &pieceFallout;

		e->x += (self->w - e->w) / 2;
		e->y += (self->w - e->w) / 2;

		e->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->head = self;
	}

	self->endX = self->damage;

	self->damage = 0;

	self->health = 0;

	self->dirX = 0;

	self->mental = 1;

	self->flags &= ~FLY;

	self->flags |= NO_DRAW;

	self->takeDamage = NULL;

	self->action = &dieWait;

	self->creditsAction = &dieWait;

	self->thinkTime = 120;
}

static void dieWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental = 0;

		self->thinkTime = 9;

		self->action = &reform;

		self->creditsAction = &reform;
	}
}

static void reform()
{
	if (self->health == -1)
	{
		increaseKillCount();

		dropRandomItem(self->x + self->w / 2, self->y);

		self->action = &entityDieVanish;
	}

	if (self->thinkTime == 0)
	{
		self->thinkTime = 30;

		self->action = &reformFinish;

		self->creditsAction = &creditsReformFinish;
	}
}

static void reformFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->health = self->maxHealth;

		self->flags &= ~NO_DRAW;

		self->action = &lookForPlayer;

		facePlayer();

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->damage = 1;

		self->takeDamage = &redTakeDamage;

		setEntityAnimation(self, "STAND");

		self->creditsAction = &creditsMove;
	}
}

static void pieceWait()
{
	if (self->head->health == -1)
	{
		self->action = &entityDieNoDrop;
	}

	else if (self->head->mental == 0)
	{
		self->action = &pieceReform;

		self->creditsAction = &pieceReform;

		if (self->face == LEFT)
		{
			self->targetX = self->head->x + self->head->w - self->w - self->offsetX;
		}

		else
		{
			self->targetX = self->head->x + self->offsetX;
		}

		self->targetY = self->head->y + self->offsetY;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= 4;
		self->dirY *= 4;

		self->flags |= FLY;

		self->touch = NULL;
	}

	if ((self->flags & ON_GROUND) && !(self->flags & FLY))
	{
		self->dirX = 0;
	}

	checkToMap(self);
}

static void pieceReform()
{
	if (atTarget())
	{
		if (self->mental == 0)
		{
			self->mental = 1;

			self->head->thinkTime--;
		}

		else
		{
			if (self->head->health == self->head->maxHealth)
			{
				self->inUse = FALSE;
			}
		}
	}

	else
	{
		self->x += self->dirX;
		self->y += self->dirY;
	}
}

static void pieceFallout()
{
	self->head->health = -1;

	self->inUse = FALSE;
}

static void redTakeDamage(Entity *other, int damage)
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

			if (self->pain != NULL)
			{
				self->pain();
			}
		}

		else
		{
			self->animationCallback = NULL;

			self->damage = 0;

			self->die();
		}
	}
}

static void creditsMove()
{
	float dirX;

	self->face = RIGHT;

	if (self->maxThinkTime == 1)
	{
		self->dirX = self->speed;
	}

	if (self->offsetX != 0)
	{
		if (self->maxThinkTime == 0)
		{
			self->maxThinkTime = 1;

			playSoundToMap("sound/enemy/centurion/walk", -1, self->x, self->y, 0);
		}

		self->dirX = 0;
	}

	else
	{
		self->maxThinkTime = 0;
	}

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->inUse = FALSE;
	}
}

static void redCreditsMove()
{
	float dirX;

	self->face = RIGHT;

	if (self->maxThinkTime == 1)
	{
		self->dirX = self->speed;
	}

	if (self->offsetX != 0)
	{
		if (self->maxThinkTime == 0)
		{
			self->thinkTime++;

			self->maxThinkTime = 1;

			playSoundToMap("sound/enemy/centurion/walk", -1, self->x, self->y, 0);
		}

		self->dirX = 0;
	}

	else
	{
		self->maxThinkTime = 0;
	}

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->inUse = FALSE;
	}

	if (self->thinkTime >= 10)
	{
		self->creditsAction = &redDie;
	}
}

static void creditsReformFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->health = self->maxHealth;

		self->flags &= ~NO_DRAW;

		self->face = RIGHT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		setEntityAnimation(self, "STAND");

		self->creditsAction = &creditsMove;
	}
}
