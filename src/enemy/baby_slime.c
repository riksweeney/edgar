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
#include "../graphics/animation.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;
extern Game game;

static void stickToTarget(void);
static void attack(void);
static void grab(Entity *other);
static void fallOff(void);
static void findPrey(void);
static void stickToTargetAndDrain(void);
static void fallOffWait(void);
static void creditsMove(void);

Entity *addBabySlime(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Baby Slime");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &attack;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDieNoDrop;
	e->pain = NULL;
	e->reactToBlock = NULL;
	e->touch = &grab;

	e->creditsAction = &creditsMove;

	if (strcmpignorecase(name, "enemy/purple_baby_slime") == 0)
	{
		e->takeDamage = &entityTakeDamageNoFlinch;
	}

	else
	{
		e->takeDamage = &entityTakeDamageFlinch;
	}

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void attack()
{
	int channel;
	long onGround = (self->flags & ON_GROUND);

	if (self->target == NULL || (self->target->type != PLAYER && self->target->health <= 0))
	{
		findPrey();
	}

	faceTarget();

	if ((self->flags & ON_GROUND) && (prand() % 30 == 0))
	{
		channel = 3 + (prand() % 3);

		if (prand() % 3 == 0)
		{
			playSoundToMap("sound/enemy/jumping_slime/baby_jump2", channel, self->x, self->y, 0);
		}

		else
		{
			playSoundToMap("sound/enemy/jumping_slime/baby_jump1", channel, self->x, self->y, 0);
		}

		self->dirX = (self->face == LEFT ? -self->speed : self->speed);

		self->dirY = -(8 + prand() % 4);
	}

	checkToMap(self);

	if (onGround == 0 && ((self->flags & ON_GROUND) || (self->standingOn != NULL)))
	{
		self->dirX = 0;
	}

	if (self->mental == 0)
	{
		self->thinkTime--;
	}

	if (self->thinkTime <= 0)
	{
		self->touch = NULL;

		self->die();
	}
}

static void grab(Entity *other)
{
	if (self->health <= 0)
	{
		return;
	}

	if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}
	}

	else if (other->type == PROJECTILE && other->parent != self)
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}

		other->inUse = FALSE;
	}

	else if (self->target == other && !(self->flags & GRABBING))
	{
		self->startX = (prand() % (other->w / 2)) * (prand() % 2 == 0 ? 1 : -1);

		self->startY = prand() % (other->h - self->h);

		setCustomAction(other, &slowDown, 3, 1, 0);

		if (strcmpignorecase(self->name, "enemy/baby_slime") == 0)
		{
			self->action = &stickToTarget;
		}

		else
		{
			self->action = &stickToTargetAndDrain;
		}

		self->touch = NULL;

		self->flags |= GRABBING;

		self->layer = FOREGROUND_LAYER;

		if (other->type == PLAYER)
		{
			other->flags |= GRABBED;
		}

		self->thinkTime = 0;

		self->mental = 3 + (prand() % 3);
	}
}

static void stickToTarget()
{
	setCustomAction(self->target, &slowDown, 3, 0, 0);

	if (self->target->type == PLAYER && self->target->health > 0)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Quickly turn left and right to shake off the slimes!"));
	}

	self->x = self->target->x + (self->target->w - self->w) / 2 + self->startX;
	self->y = self->target->y + self->startY;

	self->thinkTime++;

	if (self->face != self->target->face)
	{
		self->face = self->target->face;

		if (self->thinkTime <= 15)
		{
			self->mental--;
		}

		self->thinkTime = 0;
	}

	if (self->mental <= 0)
	{
		self->dirX = self->speed * 2 * (prand() % 2 == 0 ? -1 : 1);

		self->dirY = -6;

		setCustomAction(self->target, &slowDown, 3, -1, 0);

		self->action = &fallOff;

		self->target->flags &= ~GRABBED;
	}
}

static void stickToTargetAndDrain()
{
	Entity *temp;

	if (self->target->type == PLAYER && self->target->health > 0)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Quickly turn left and right to shake off the slimes!"));

		setCustomAction(self->target, &slowDown, 3, 0, 0);
	}

	/* Fall off immediately if boss has armour */

	else if (self->target->mental != 0)
	{
		if (prand() % 10 == 0)
		{
			setInfoBoxMessage(120, 255, 255, 255, _("The armour is too tough for them to penetrate..."));
		}

		self->mental = 0;
	}

	else if (self->target->maxThinkTime == 99)
	{
		self->mental = 0;
	}

	self->x = self->target->x + (self->target->w - self->w) / 2 + self->startX;
	self->y = self->target->y + self->startY;

	if (self->target->health > 0)
	{
		self->thinkTime++;

		if (self->face != self->target->face)
		{
			self->face = self->target->face;

			if (self->thinkTime <= 15)
			{
				self->mental--;
			}

			self->thinkTime = 0;
		}

		if (self->thinkTime >= 60)
		{
			temp = self;

			self = self->target;

			self->takeDamage(temp, 1);

			self = temp;

			self->thinkTime = self->target == PLAYER ? 0 : 45;
		}
	}

	else
	{
		self->mental = 180 + (prand() % 420);

		self->action = &fallOffWait;
	}

	if (self->mental <= 0)
	{
		self->dirX = self->speed * 2 * (prand() % 2 == 0 ? -1 : 1);

		self->dirY = -6;

		if (self->target->type == PLAYER)
		{
			setCustomAction(self->target, &slowDown, 3, -1, 0);
		}

		self->action = &fallOff;

		self->target->flags &= ~GRABBED;
	}
}

static void fallOffWait()
{
	setCustomAction(self->target, &slowDown, 3, 0, 0);

	self->mental--;

	self->x = self->target->x + (self->target->w - self->w) / 2 + self->startX;
	self->y = self->target->y + self->startY;

	if (self->mental <= 0)
	{
		self->dirX = self->speed * 2 * (prand() % 2 == 0 ? -1 : 1);

		self->dirY = -6;

		setCustomAction(self->target, &slowDown, 3, -1, 0);

		self->action = &fallOff;

		self->target->flags &= ~GRABBED;
	}
}

static void fallOff()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->die();
	}
}

static void findPrey()
{
	Entity *e = getEntityByObjectiveName("ARMOUR_BOSS");

	self->target = (e == NULL ? &player : e);
}

static void creditsMove()
{
	int channel;
	float dirX;
	long onGround = (self->flags & ON_GROUND);

	if (self->flags & ON_GROUND)
	{
		channel = 3 + (prand() % 3);

		if (prand() % 3 == 0)
		{
			playSoundToMap("sound/enemy/jumping_slime/baby_jump2", channel, self->x, self->y, 0);
		}

		else
		{
			playSoundToMap("sound/enemy/jumping_slime/baby_jump1", channel, self->x, self->y, 0);
		}

		self->dirX = self->speed;

		self->dirY = -(8 + prand() % 4);
	}

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->inUse = FALSE;
	}

	if (onGround == 0 && ((self->flags & ON_GROUND) || (self->standingOn != NULL)))
	{
		self->dirX = 0;
	}
}
