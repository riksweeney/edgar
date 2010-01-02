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
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../audio/music.h"
#include "../collisions.h"
#include "../game.h"
#include "../player.h"
#include "../graphics/decoration.h"
#include "../system/error.h"

extern Entity *self;

static void wait(void);
static void drop(void);
static void touch(Entity *);
static void initialise(void);
static void chasePlayer(void);
static void addDust(void);
static void die(void);
static void fallout(void);

Entity *addBoulderBoss2(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Boulder Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->die = &die;
	e->fallout = &fallout;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	if (self->active == TRUE)
	{
		fadeOutMusic(500);

		self->x = self->startX;
		self->y = self->startY;

		self->touch = &touch;

		self->flags &= ~(NO_DRAW|FLY);

		self->flags |= ATTACKING;

		self->action = &drop;
	}
}

static void drop()
{
	if (self->flags & ON_GROUND)
	{
		self->thinkTime = 30;

		playSoundToMap("sound/common/crash.ogg", -1, self->x, self->y, 0);

		shakeScreen(STRONG, self->thinkTime / 2);

		addDust();

		self->action = &wait;
	}

	checkToMap(self);
}

static void wait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		playBossMusic();

		playSoundToMap("sound/boss/boulder_boss/roll.ogg", BOSS_CHANNEL, self->x, self->y, -1);

		setEntityAnimation(self, WALK);

		self->action = &chasePlayer;
		
		self->thinkTime = 1;
	}
}

static void chasePlayer()
{
	long onGround = self->flags & ON_GROUND;
	
	self->thinkTime--;
	
	if (self->thinkTime > 0)
	{
		checkToMap(self);
	}
	
	else if (self->thinkTime == 0)
	{
		facePlayer();
		
		self->frameSpeed = 12;
		
		checkToMap(self);
	}
	
	else
	{
		self->dirX = self->face == LEFT ? -2.0f : 2.0f;
		
		checkToMap(self);
		
		if (!(self->flags & ON_GROUND))
		{
			self->mental++;
		}
		
		if (onGround == 0 && (self->flags & ON_GROUND))
		{
			if (self->mental > 15)
			{
				playSoundToMap("sound/common/crash.ogg", -1, self->x, self->y, 0);

				shakeScreen(STRONG, self->thinkTime / 2);

				addDust();
				
				self->thinkTime = 30;
				
				self->dirX = 0;
				
				self->frameSpeed = 0;
			}
			
			self->mental = 0;
		}
	}
}

static void touch(Entity *other)
{
	Entity *temp = self;

	if (other->die != NULL)
	{
		self = other;

		self->die();

		self = temp;
	}
}

static void addDust()
{
	int i;

	for (i=0;i<25;i++)
	{
		addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
	}
}

static void fallout()
{
	self->element = FIRE;
	
	self->dirX = 0;
	
	self->dirY = 0;
	
	self->frameSpeed = 0;
	
	self->flags |= DO_NOT_PERSIST;
	
	self->action = &die;
	
	fadeBossMusic();
	
	stopSound(BOSS_CHANNEL);
}
	
static void die()
{
	self->dirY = 0.5;
	
	checkToMap(self);
	
	if (self->flags & ON_GROUND)
	{
		self->inUse = FALSE;
	}
}
