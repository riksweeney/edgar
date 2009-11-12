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
#include "../entity.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../system/error.h"
#include "../player.h"

extern Entity *self;

static void move(void);
static void wait(void);
static void touch(Entity *);
static void suckIn(void);

Entity *addWhirlwind(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Whirlwind");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &moveLeftToRight;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->touch = &touch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void move()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 60;

		self->dirX = 0;

		self->action = &wait;
	}

	else
	{
		moveLeftToRight();
	}
}

static void wait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 120 + prand() % 120;

		self->action = &move;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	if (other->health > 0 && self->health > 0 && other->type == PLAYER && !(other->flags & INVULNERABLE))
	{
		if (self->target != NULL)
		{
			self->target->inUse = FALSE;
		}

		self->target = other;

		self->dirX = 0;

		self->thinkTime = 60;

		other->dirX = 0;
		other->dirY = 0;

		setCustomAction(other, &helpless, 60, 0);
		setCustomAction(other, &invulnerableNoFlash, 60, 0);

		self->action = &suckIn;

		self->targetX = self->x + self->w / 2 - other->w / 2;
	}

	else if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		/* Take very little damage when in a whirlwind */

		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage > 2 ? 2 : other->damage);
		}
	}

	else if (other->type == PROJECTILE && other->parent != self && other->parent->type != ENEMY)
	{
		/* Reflect the projectile */

		other->face = other->face == LEFT ? RIGHT : LEFT;

		other->damage = 2;

		other->dirX *= -1;

		other->parent = self;
	}
}

static void suckIn()
{
	Entity *e;

	if (fabs(self->target->x - self->targetX) > fabs(4))
	{
		self->target->x += self->target->x > self->targetX ? -4 : 4;

		setCustomAction(self->target, &helpless, 60, 0);
		setCustomAction(self->target, &invulnerableNoFlash, 60, 0);
	}

	else
	{
		self->target->flags |= NO_DRAW;

		self->target->x = self->targetX;

		self->target->y = self->y;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			e = removePlayerWeapon();

			if (e != NULL)
			{
				e->x = self->x;
				e->y = self->y;

				e->dirX = 8 * (prand() % 2 == 0 ? -1 : 1);
				e->dirY = -12;

				setCustomAction(e, &invulnerable, 120, 0);
			}

			e = removePlayerShield();

			if (e != NULL)
			{
				e->x = self->x;
				e->y = self->y;

				e->dirX = 8 * (prand() % 2 == 0 ? -1 : 1);
				e->dirY = -12;

				setCustomAction(e, &invulnerable, 120, 0);
			}

			setCustomAction(self->target, &invulnerable, 60, 0);

			setPlayerStunned(30);

			self->target->dirX = 6 * (prand() % 2 == 0 ? -1 : 1);
			self->target->dirY = -8;

			self->target->flags &= ~NO_DRAW;

			self->target = NULL;

			self->thinkTime = 180 + prand() % 180;

			self->action = &move;
		}
	}
}
