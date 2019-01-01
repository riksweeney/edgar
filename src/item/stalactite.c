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
#include "../enemy/rock.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void touch(Entity *);
static void takeDamage(Entity *, int);
static void die(void);
static void respawn(void);
static void entityWait(void);
static void fallout(void);
static void falloutPause(void);

Entity *addStalactite(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Stalactite");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &entityWait;
	e->touch = &touch;
	e->die = &die;
	e->fallout = &fallout;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void touch(Entity *other)
{
	if (self->active == TRUE && (other->flags & ATTACKING) && !(self->flags & INVULNERABLE))
	{
		takeDamage(other, other->damage);
	}

	if (self->inUse == TRUE && !(self->flags & ATTACKING))
	{
		pushEntity(other);
	}
}

static void fallout()
{
	if (self->mental == 0 && (self->environment == AIR || self->environment == LAVA))
	{
		self->thinkTime = 60;

		self->action = &falloutPause;
	}

	else if (self->mental < 0)
	{
		self->action = &die;
	}
}

static void falloutPause()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &die;
	}

	checkToMap(self);
}

static void entityWait()
{
	int i;
	long onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (self->mental < 0)
		{
			self->thinkTime = 300;

			self->action = &die;
		}

		else if (onGround == 0 && self->environment == AIR)
		{
			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}

			playSoundToMap("sound/enemy/red_grub/thud", -1, self->x, self->y, 0);
		}
	}
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (strcmpignorecase(self->requires, other->name) == 0)
	{
		self->health -= damage;

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		if (self->health <= 0)
		{
			if (self->thinkTime == 0)
			{
				self->flags &= ~FLY;

				self->health = self->maxHealth;

				if (self->mental < 0)
				{
					self->flags |= ATTACKING;
				}

				self->thinkTime = 1;
			}

			else
			{
				self->die();
			}
		}
	}

	else
	{
		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		playSoundToMap("sound/common/dink", -1, self->x, self->y, 0);

		if (other->reactToBlock != NULL)
		{
			temp = self;

			self = other;

			self->reactToBlock(temp);

			self = temp;
		}

		if (other->type != PROJECTILE && prand() % 10 == 0)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
		}

		damage = 0;
	}
}

static void die()
{
	Entity *e;

	playSoundToMap("sound/common/crumble", -1, self->x, self->y, 0);

	self->flags |= NO_DRAW;

	self->touch = NULL;

	self->thinkTime = 300;

	self->action = &respawn;

	e = addSmallRock(self->x, self->y, "common/small_rock");

	e->x += (self->w - e->w) / 2;
	e->y += (self->h - e->h) / 2;

	e->dirX = -3;
	e->dirY = -8;

	e = addSmallRock(self->x, self->y, "common/small_rock");

	e->x += (self->w - e->w) / 2;
	e->y += (self->h - e->h) / 2;

	e->dirX = 3;
	e->dirY = -8;
}

static void respawn()
{
	float x, y;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental == -2)
		{
			self->inUse = FALSE;
		}

		x = self->x;

		y = self->y;

		self->x = self->startX;

		self->y = self->startY;

		if (isSpaceEmpty(self) != NULL)
		{
			self->x = x;

			self->y = y;

			self->thinkTime = 60;
		}

		else
		{
			self->flags &= ~NO_DRAW;

			self->flags |= FLY;

			self->action = &entityWait;

			self->health = self->maxHealth;

			self->touch = &touch;

			setCustomAction(self, &invulnerable, 180, 0, 0);

			self->x = self->startX;

			self->y = self->startY;

			self->dirX = self->dirY = 0;

			self->flags &= ~ATTACKING;
		}
	}
}
