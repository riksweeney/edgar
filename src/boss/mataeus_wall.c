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
#include "../graphics/decoration.h"
#include "../map.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void entityWait(void);
static void touch(Entity *);
static void initFall(void);
static void resetWait(void);
static void resetPlatform(void);
static void touch(Entity *);
static void lavaWait(void);
static void lavaResetMove(void);
static void lavaResetWait(void);
static void wallAttack(void);
static void wallAttackWait(void);

Entity *addMataeusWall(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Mataeus Wall");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &entityWait;
	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	checkToMap(self);

	if (self->mental != 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->thinkTime = 60;

			self->action = &initFall;
		}
	}
}

static void initFall()
{
	int i;
	int tileX, tileY;

	self->thinkTime--;

	if (self->mental == 0 && (self->flags & FLY))
	{
		self->x = self->startX;

		self->action = &entityWait;
	}

	else if (self->thinkTime > 0)
	{
		if (self->x == self->startX || (self->thinkTime % 4 == 0))
		{
			self->x = self->startX + (3 * (self->x < self->startX ? 1 : -1));
		}
	}

	else
	{
		self->flags &= ~FLY;

		self->thinkTime = 0;

		self->x = self->startX;

		self->damage = 1;

		checkToMap(self);

		if (self->flags & ON_GROUND)
		{
			playSoundToMap("sound/enemy/red_grub/thud", -1, self->x, self->y, 0);

			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}

			self->damage = 0;

			self->thinkTime = 30;

			switch (self->mental)
			{
				case 0:
				case 1:
					self->action = &resetWait;
				break;

				default:
					self->action = &lavaWait;
				break;
			}

			if (self->mental == 2)
			{
				self->flags |= FLY;

				tileX = (self->x + 16) / TILE_SIZE;
				tileY = (self->y + 16) / TILE_SIZE;

				self->targetX = tileX;
				self->targetY = tileY;

				for (i=0;i<3;i++)
				{
					tileY++;

					setTileAt(tileX, tileY, LAVA_TILE_START);
				}
			}
		}
	}
}

static void lavaWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &lavaResetMove;
	}

	checkToMap(self);
}

static void resetWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= FLY;

		self->action = &resetPlatform;
	}

	checkToMap(self);
}

static void resetPlatform()
{
	self->dirY = -3;

	checkToMap(self);

	if (self->y <= self->startY)
	{
		self->y = self->startY;

		self->dirY = 0;

		self->thinkTime = self->maxThinkTime;

		self->action = &entityWait;

		self->mental = 0;
	}
}

static void lavaResetMove()
{
	self->dirY = -3;

	checkToMap(self);

	if (self->y <= self->startY + 256)
	{
		self->y = self->startY + 256;

		self->dirY = 0;

		self->head->mental--;

		self->thinkTime = 60 + prand() % 300;

		self->action = &lavaResetWait;
	}
}

static void lavaResetWait()
{
	int i, tileX, tileY;

	if (self->mental == 3 || self->mental == 4)
	{
		if (self->mental == 3)
		{
			tileX = self->targetX;
			tileY = self->targetY;

			for (i=0;i<3;i++)
			{
				tileY++;

				setTileAt(tileX, tileY, i == 0 ? 4 : 1 + prand() % 3);
			}

			self->mental = 4;
		}

		self->thinkTime--;

		if (self->thinkTime > 0 && self->thinkTime <= 30)
		{
			if (self->x == self->startX || (self->thinkTime % 4 == 0))
			{
				self->x = self->startX + (3 * (self->x < self->startX ? 1 : -1));
			}
		}

		else if (self->thinkTime <= 0)
		{
			self->x = self->startX;

			self->dirY = -9;

			if (self->y <= self->startY)
			{
				self->head->mental--;

				self->y = self->startY;

				self->thinkTime = self->maxThinkTime;

				self->action = &entityWait;

				self->mental = 0;
			}
		}
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	int bottomBefore;
	float dirX;
	Entity *temp;

	if (other->type == PLAYER && self->damage != 0)
	{
		temp = self;

		self = other;

		self->takeDamage(temp, temp->damage);

		self = temp;
	}

	if (other->type == PLAYER && other->dirY > 0)
	{
		/* Trying to move down */

		if (collision(other->x, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			bottomBefore = other->y + other->h - other->dirY - 1;

			if (abs(bottomBefore - self->y) < self->h - 1)
			{
				if (self->dirY < 0)
				{
					other->y -= self->dirY;

					other->dirY = self->dirY;

					dirX = other->dirX;

					other->dirX = 0;

					checkToMap(other);

					other->dirX = dirX;

					if (other->dirY == 0)
					{
						/* Gib the player */

						temp = self;

						self = other;

						freeEntityList(playerGib());

						self = temp;
					}
				}

				/* Place the player as close to the solid tile as possible */

				other->y = self->y;
				other->y -= other->h;

				other->standingOn = self;
				other->dirY = 0;
				other->flags |= ON_GROUND;

				if (self->mental == 2 && self->head->mental == 0)
				{
					self->thinkTime = 90;

					self->mental = 5;

					self->action = &wallAttackWait;
				}
			}
		}
	}
}

static void wallAttackWait()
{
	self->thinkTime--;

	if (self->thinkTime > 0 && self->thinkTime <= 90)
	{
		if (self->x == self->startX || (self->thinkTime % 4 == 0))
		{
			self->x = self->startX + (3 * (self->x < self->startX ? 1 : -1));
		}
	}

	else if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "ATTACK_1");

		playSoundToMap("sound/item/tesla_electrocute", -1, self->x, self->y, 0);

		self->x = self->startX;

		self->thinkTime = 30;

		self->damage = 1;

		self->action = &wallAttack;
	}

	checkToMap(self);
}

static void wallAttack()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		self->damage = 0;

		self->mental = self->mental == 3 ? 3 : 2;

		self->thinkTime = 60 + prand() % 180;

		self->action = &lavaResetWait;
	}
}
