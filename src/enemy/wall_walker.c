/*
Copyright (C) 2009-2010 Parallel Realities

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
#include "../system/properties.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../system/error.h"
#include "../graphics/decoration.h"
#include "../audio/audio.h"

extern Entity *self, player;

static void walkOnGround(void);
static void walkUpWall(void);
static void walkOnCeiling(void);
static void dropOnPlayer(void);
static void dropWait(void);

Entity *addWallWalker(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Wall Walker");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &walkOnGround;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void walkOnGround()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->flags |= FLY;

		self->action = &walkUpWall;

		self->dirY = -self->speed;
	}
}

static void walkUpWall()
{
	checkToMap(self);

	if (self->dirY == 0)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->action = &walkOnCeiling;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}
}

static void walkOnCeiling()
{
	checkToMap(self);

	if (self->dirX == 0 || isAtCeilingEdge(self) == TRUE)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	if (player.health > 0 &&
		collision(self->x, self->y, self->w, SCREEN_HEIGHT, player.x ,player.y, player.w, player.h) == 1)
	{
		self->dirX = 0;

		self->thinkTime = 30;

		self->action = &dropOnPlayer;
	}
}

static void dropOnPlayer()
{
	int i;
	long onGround = self->flags & ON_GROUND;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~FLY;
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (onGround == 0)
		{
			playSoundToMap("sound/enemy/red_grub/thud.ogg", -1, self->x, self->y, 0);

			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}
		}

		self->action = &dropWait;

		self->thinkTime = 60;
	}
}

static void dropWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->action = &walkOnGround;
	}

	checkToMap(self);
}
