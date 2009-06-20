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
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "enemies.h"

extern Entity *self, player;

static void die(void);
static void wait(void);
static void purpleWait(void);
static void attack(void);
static void purpleAttack(void);
static void swim(void);
static void jumpOut(void);
static void fallout(void);
static void layEgg(void);

Entity *addJumpingSlime(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Jumping Slime\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	if (strcmpignorecase(name, "enemy/purple_jumping_slime") == 0)
	{
		e->action = &purpleWait;

		e->fallout = &fallout;
	}

	else
	{
		e->action = &wait;
	}

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void die()
{
	entityDie();
}

static void wait()
{
	if (prand() % 8 == 0)
	{
		if (collision(self->x - 160, self->y, 320 + self->w, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &attack;

			self->thinkTime = 0;
		}
	}

	checkToMap(self);
}

static void purpleWait()
{
	if (prand() % 4 == 0)
	{
		if (collision(self->x - 240, self->y, 480 + self->w, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &purpleAttack;

			self->thinkTime = 0;
		}
	}

	checkToMap(self);
}

static void attack()
{
	int onGround = (self->flags & ON_GROUND);

	if (self->flags & ON_GROUND)
	{
		self->face = player.x < self->x ? LEFT : RIGHT;
		
		self->dirX = 0;

		if (self->thinkTime == 0)
		{
			self->dirY = -(6 + prand() % 2);

			self->dirX = (prand() % 2 + 2) * (prand() % 2 == 0 ? -1 : 1);
		}

		else
		{
			self->thinkTime--;
		}
	}

	checkToMap(self);

	if (onGround == 0 && self->flags & ON_GROUND)
	{
		self->thinkTime = 30 + prand () % 60;
	}
}

static void purpleAttack()
{
	int onGround = (self->flags & ON_GROUND);

	if (self->flags & ON_GROUND)
	{
		self->face = player.x < self->x ? LEFT : RIGHT;
		
		self->dirX = 0;

		if (self->thinkTime == 0)
		{
			self->dirY = -(6 + prand() % 2);

			self->dirX = (prand() % 2 + 2) * (prand() % 2 == 0 ? -1 : 1);

			if (prand() % 8 == 0)
			{
				layEgg();
			}
		}

		else
		{
			self->thinkTime--;
		}
	}

	checkToMap(self);

	if (onGround == 0 && self->flags & ON_GROUND)
	{
		self->thinkTime = 30 + prand () % 60;
	}
}

static void swim()
{
	if (prand() % 30 == 0)
	{
		if (prand() % 5 == 0 && collision(self->x - 160, self->y - 128, 320 + self->w, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &jumpOut;
		}

		else
		{
			self->dirX = prand() % 2 == 0 ? -self->speed : self->speed;

			self->dirY = prand() % 2 == 0 ? -self->speed / 2 : self->speed / 4;
		}
	}

	checkToMap(self);

	if (self->environment == AIR)
	{
		self->flags &= ~FLY;

		self->dirX = 0;

		self->action = &purpleAttack;
	}
}

static void jumpOut()
{
	/* Jump towards the player */

	if (self->environment == WATER)
	{
		self->dirY = -14;
	}

	self->dirX = player.x < self->x ? -self->speed : self->speed;

	checkToMap(self);

	if (self->environment == AIR)
	{
		self->flags &= ~FLY;

		if (self->flags & ON_GROUND)
		{
			self->dirX = 0;

			self->action = &purpleAttack;
		}
	}
}

static void fallout()
{
	if (self->environment == WATER)
	{
		self->flags |= FLY;

		self->action = &swim;
	}

	else
	{
		entityDie();
	}
}

static void layEgg()
{
	int count, i;
	Entity *e;

	count = 1 + (prand() % 5);

	for (i=0;i<count;i++)
	{
		e = addEnemy("enemy/jumping_slime_egg", 0, 0);

		e->x = self->x + (self->w - e->w) / 2;
		e->y = self->y;

		e->x += (prand() % 20) * (prand() % 2 == 0 ? 1 : -1);

		e->startX = e->x;
		e->startY = e->y;
	}
}
