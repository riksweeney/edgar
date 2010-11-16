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
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../system/error.h"
#include "../player.h"
#include "../game.h"
#include "../item/key_items.h"

extern Entity *self, player;
extern Game game;

static void entityWait(void);
static void explodeInit(void);
static void explode(void);
static void touch(Entity *);
static void throwGazerEye(int);
static void explodeWait(void);

Entity *addExplodingGazerEyeDud(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Exploding Gazer Eye Dud");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &entityWait;
	e->activate = &throwGazerEye;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void entityWait()
{
	checkToMap(self);
	
	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
		
		if (self->active == FALSE)
		{
			self->touch = &keyItemTouch;
		}
		
		else
		{
			self->touch = &touch;
			
			self->thinkTime = 30;
		}
	}
}

static void throwGazerEye(int val)
{
	Entity *e;
	
	if (game.status == IN_GAME)
	{
		setEntityAnimation(self, STAND);

		self->active = TRUE;

		e = addEntity(*self, player.x + (player.face == RIGHT ? player.w : 0), player.y);

		e->touch = &touch;

		e->action = &explodeWait;
		
		e->fallout = &entityDieNoDrop;
		
		e->flags |= DO_NOT_PERSIST;

		e->dirX = player.face == LEFT ? -8 : 8;

		e->dirY = ITEM_JUMP_HEIGHT;

		playSoundToMap("sound/common/throw.ogg", -1, player.x, player.y, 0);

		self->inUse = FALSE;
	}	
}

static void explodeWait()
{
	checkToMap(self);
	
	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
		
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, WALK);
			
			self->thinkTime = 15;
			
			self->action = &explodeInit;
		}
	}
}

static void explodeInit()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->thinkTime = 15;
		
		setEntityAnimation(self, JUMP);
		
		self->action = &explode;
	}
}

static void explode()
{
	Entity *temp;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		playSoundToMap("sound/enemy/gazer/flash.ogg", -1, self->x, self->y, 0);

		fadeFromColour(255, 255, 255, 60);
		
		if (self->head != NULL)
		{
			temp = self;
			
			self = self->head;
			
			self->activate(100);
			
			self = temp;
		}
		
		self->inUse = FALSE;
	}
}

static void touch(Entity *other)
{
	
}
