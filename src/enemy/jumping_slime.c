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
#include "../entity.h"
#include "../graphics/animation.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "enemies.h"

extern Entity *self, player;

static void redDie(void);
static void die(void);
static void entityWait(void);
static void purpleWait(void);
static void attack(void);
static void purpleAttack(void);
static void swim(void);
static void jumpOut(void);
static void fallout(void);
static void layEgg(void);
static void dropPurpleSlimes(void);
static void creditsMove(void);
static void creditsPurpleMove(void);
static void creditsRedMove(void);
static void creditsDie(void);
static void reactToBlock(Entity *);

Entity *addJumpingSlime(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Jumping Slime");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	if (strcmpignorecase(name, "enemy/purple_jumping_slime") == 0)
	{
		e->action = &purpleWait;

		e->fallout = &fallout;

		e->die = &die;

		e->creditsAction = &creditsPurpleMove;
	}

	else if (strcmpignorecase(name, "enemy/red_jumping_slime") == 0)
	{
		e->action = &entityWait;

		e->fallout = &fallout;

		e->die = &redDie;

		e->creditsAction = &creditsRedMove;
	}

	else
	{
		e->action = &entityWait;

		e->die = &die;

		e->creditsAction = &creditsMove;
	}

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &reactToBlock;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void die()
{
	playSoundToMap("sound/enemy/jumping_slime/slime_die", -1, self->x, self->y, 0);

	entityDie();
}

static void redDie()
{
	int i;
	Entity *e;

	self->takeDamage = NULL;

	self->damage = 0;

	setEntityAnimation(self, "DIE");

	self->mental = 5 + prand() % 15;

	if (prand() % 2 == 0)
	{
		for (i=0;i<self->mental;i++)
		{
			e = addEnemy("enemy/purple_baby_slime", self->x, self->y);

			e->x += self->w / 2 - e->w / 2;
			e->y += self->h / 2 - e->h / 2;

			e->dirX = (prand() % 40) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = -8;

			e->dirX /= 10;
		}

		self->mental = 0;
	}

	playSoundToMap("sound/enemy/jumping_slime/slime_die", -1, self->x, self->y, 0);

	self->thinkTime = 60 + prand() % 60;

	self->action = &dropPurpleSlimes;

	self->creditsAction = &dropPurpleSlimes;

	self->flags |= DO_NOT_PERSIST;

	checkToMap(self);
}

static void dropPurpleSlimes()
{
	int i;
	Entity *e;

	checkToMap(self);

	if ((self->flags & ON_GROUND) || self->standingOn != NULL)
	{
		self->dirX = 0;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		for (i=0;i<self->mental;i++)
		{
			e = addEnemy("enemy/purple_baby_slime", self->x, self->y);

			e->x += self->w / 2 - e->w / 2;
			e->y += self->h / 2 - e->h / 2;

			e->dirX = (prand() % 20) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = ITEM_JUMP_HEIGHT;

			e->dirX /= 10;
		}

		self->action = &entityDie;

		self->creditsAction = &creditsDie;
	}
}

static void entityWait()
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
	if (self->flags & ON_GROUND)
	{
		facePlayer();

		self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;

		if (self->thinkTime == 0)
		{
			if (prand() % 3 == 0)
			{
				playSoundToMap("sound/enemy/jumping_slime/jump2", -1, self->x, self->y, 0);
			}

			else
			{
				playSoundToMap("sound/enemy/jumping_slime/jump1", -1, self->x, self->y, 0);
			}

			self->dirY = -(6 + prand() % 2);

			if (prand() % 2 == 0)
			{
				self->face = LEFT;

				if (isAtEdge(self) == TRUE)
				{
					self->face = RIGHT;
				}
			}

			else
			{
				self->face = RIGHT;

				if (isAtEdge(self) == TRUE)
				{
					self->face = LEFT;
				}
			}

			self->dirX = (prand() % 2 + 2) * (self->face == LEFT ? -1 : 1);

			self->thinkTime = 30 + prand() % 60;
		}

		else
		{
			self->thinkTime--;
		}
	}

	else
	{
		if (self->dirX == 0)
		{
			self->dirX = self->face == LEFT ? 2 : -2;
		}
	}

	checkToMap(self);
}

static void purpleAttack()
{
	if (self->flags & ON_GROUND)
	{
		facePlayer();

		self->dirX = self->standingOn == NULL ? 0 : self->standingOn->dirX;

		if (self->thinkTime == 0)
		{
			if (prand() % 3 == 0)
			{
				playSoundToMap("sound/enemy/jumping_slime/jump2", -1, self->x, self->y, 0);
			}

			else
			{
				playSoundToMap("sound/enemy/jumping_slime/jump1", -1, self->x, self->y, 0);
			}

			self->dirY = -(6 + prand() % 2);

			if (prand() % 2 == 0)
			{
				self->face = LEFT;

				if (isAtEdge(self) == TRUE)
				{
					self->face = RIGHT;
				}
			}

			else
			{
				self->face = RIGHT;

				if (isAtEdge(self) == TRUE)
				{
					self->face = LEFT;
				}
			}

			self->dirX = (prand() % 2 + 2) * (self->face == LEFT ? -1 : 1);

			self->thinkTime = 30 + prand() % 60;

			if (player.health > 0 && (prand() % 8 == 0))
			{
				layEgg();
			}
		}

		else
		{
			self->thinkTime--;
		}
	}

	else
	{
		if (self->dirX == 0)
		{
			self->dirX = self->face == LEFT ? 2 : -2;
		}
	}

	checkToMap(self);
}

static void reactToBlock(Entity *other)
{
	self->dirX = 0;
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

		self->action = strcmpignorecase(self->name, "enemy/red_jumping_slime") == 0 ? &attack : &purpleAttack;
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

			self->action = strcmpignorecase(self->name, "enemy/red_jumping_slime") == 0 ? &attack : &purpleAttack;
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

		e->pain = &enemyPain;

		e->thinkTime = 120 + (prand() % 180);

		e->x = self->x + (self->w - e->w) / 2;
		e->y = self->y;

		e->x += (prand() % 20) * (prand() % 2 == 0 ? 1 : -1);

		e->startX = e->x;
		e->startY = e->y;
	}
}

static void creditsMove()
{
	float dirX;

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		if (self->thinkTime == 0)
		{
			self->dirY = -(6 + prand() % 2);

			self->dirX = 3;

			self->thinkTime = 30 + prand() % 30;

			if (prand() % 3 == 0)
			{
				playSoundToMap("sound/enemy/jumping_slime/jump2", -1, self->x, self->y, 0);
			}

			else
			{
				playSoundToMap("sound/enemy/jumping_slime/jump1", -1, self->x, self->y, 0);
			}
		}

		else
		{
			self->thinkTime--;
		}
	}

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->inUse = FALSE;
	}
}

static void creditsPurpleMove()
{
	int i;
	float dirX;
	Entity *e;

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		if (self->thinkTime == 0)
		{
			self->dirY = -(6 + prand() % 2);

			self->dirX = 3;

			self->thinkTime = 30 + prand() % 30;

			if (prand() % 3 == 0)
			{
				playSoundToMap("sound/enemy/jumping_slime/jump2", -1, self->x, self->y, 0);
			}

			else
			{
				playSoundToMap("sound/enemy/jumping_slime/jump1", -1, self->x, self->y, 0);
			}

			self->mental++;

			if (self->mental == 7)
			{
				for (i=0;i<10;i++)
				{
					e = addEnemy("enemy/jumping_slime_egg", 0, 0);

					e->thinkTime = 60 + prand() % 180;

					e->x = self->x + (self->w - e->w) / 2;
					e->y = self->y;

					e->x += (prand() % 20) * (prand() % 2 == 0 ? 1 : -1);

					e->startX = e->x;
					e->startY = e->y;
				}
			}
		}

		else
		{
			self->thinkTime--;
		}
	}

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->inUse = FALSE;
	}
}

static void creditsRedMove()
{
	float dirX;

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;

		if (self->thinkTime == 0)
		{
			self->dirY = -(6 + prand() % 2);

			self->dirX = 3;

			self->thinkTime = 30 + prand() % 30;

			if (prand() % 3 == 0)
			{
				playSoundToMap("sound/enemy/jumping_slime/jump2", -1, self->x, self->y, 0);
			}

			else
			{
				playSoundToMap("sound/enemy/jumping_slime/jump1", -1, self->x, self->y, 0);
			}

			self->mental++;

			if (self->mental == 10)
			{
				playSoundToMap("sound/enemy/jumping_slime/slime_die", -1, self->x, self->y, 0);

				self->die();
			}
		}

		else
		{
			self->thinkTime--;
		}
	}

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		self->inUse = FALSE;
	}
}

static void creditsDie()
{
	self->thinkTime++;

	if (self->thinkTime > 180)
	{
		self->inUse = FALSE;
	}
}
