/*
Copyright (C) 2009-2011 Parallel Realities

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

#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../custom_actions.h"
#include "../enemy/rock.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../audio/audio.h"
#include "../system/error.h"
#include "../system/random.h"
#include "../graphics/decoration.h"
#include "../hud.h"

extern Entity *self;

static void touch(Entity *);
static void floorShake(void);
static void die(void);
static void entityWait(void);
static void dieWait(void);
static void respawn(void);

Entity *addSorcerorFloor(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Sorceror Floor");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &entityWait;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->die = &die;

	setEntityAnimation(e, "STAND");

	return e;
}

static void touch(Entity *other)
{
	pushEntity(other);
	
	if (other->standingOn == self && other->type == PLAYER && self->active == TRUE)
	{
		self->mental = 1;
	}
}

static void entityWait()
{
	if (self->mental == 1)
	{
		self->flags |= PLAYER_TOUCH_ONLY;
		
		self->action = &floorShake;
	}
}

static void floorShake()
{
	self->thinkTime--;

	if (self->thinkTime > 0)
	{
		if (self->x == self->startX || (self->thinkTime % 4 == 0))
		{
			self->x = self->startX + (3 * (self->x < self->startX ? 1 : -1));
		}

		self->dirY = 0;
	}

	else
	{
		self->flags &= ~PLAYER_TOUCH_ONLY;
		
		self->die();
	}
}

static void die()
{
	Entity *e;

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

	self->flags |= NO_DRAW;

	self->touch = NULL;

	self->action = &dieWait;

	self->thinkTime = self->maxThinkTime;

	playSoundToMap("sound/common/crumble.ogg", 4, self->x, self->y, 0);
}

static void dieWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 30;
		
		self->x = self->startX;
		self->y = self->startY;

		self->action = &respawn;
	}
}

static void respawn()
{
	self->thinkTime--;

	if (self->thinkTime % 3 == 0)
	{
		self->flags ^= NO_DRAW;
	}

	if (self->thinkTime <= 0)
	{
		self->flags &= ~NO_DRAW;

		self->touch = &touch;

		self->thinkTime = self->maxThinkTime;

		self->action = &entityWait;
		
		self->mental = 0;
	}
}
