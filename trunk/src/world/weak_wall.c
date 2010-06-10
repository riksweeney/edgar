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

#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../custom_actions.h"
#include "../enemy/rock.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../audio/audio.h"
#include "../system/error.h"

extern Entity *self;

static void touch(Entity *);
static void takeDamage(Entity *, int);
static void die(void);
static void fallout(void);
static void init(void);
static void dieWait(void);
static void respawn(void);

Entity *addWeakWall(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add %s", name);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->takeDamage = &takeDamage;
	e->die = &die;
	e->fallout = &fallout;
	
	e->flags |= OBSTACLE;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	self->face = RIGHT;

	self->action = &doNothing;
}

static void touch(Entity *other)
{
	if (self->active == TRUE && (other->flags & ATTACKING) && !(self->flags & INVULNERABLE))
	{
		if (!(self->flags & NO_DRAW))
		{
			takeDamage(other, other->damage);
		}
	}
	
	if (self->inUse == TRUE)
	{
		pushEntity(other);
	}
}

static void takeDamage(Entity *other, int damage)
{
	if (damage > 100)
	{
		self->die();
	}

	else if (strcmpignorecase(self->requires, other->name) == 0)
	{
		self->health -= damage;

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);

		if (self->health <= 0)
		{
			self->die();
		}
	}
	
	else if (self->flags & NO_DRAW)
	{
		return;
	}

	else
	{
		setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);

		playSoundToMap("sound/common/dink.ogg", -1, self->x, self->y, 0);
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
	
	if (self->mental == -1)
	{
		self->flags |= NO_DRAW;
		
		self->touch = NULL;
		
		self->action = &dieWait;
		
		self->thinkTime = self->maxThinkTime;
	}
	
	else
	{
		self->inUse = FALSE;

		fireTrigger(self->objectiveName);

		fireGlobalTrigger(self->objectiveName);
	}

	playSoundToMap("sound/common/crumble.ogg", 4, self->x, self->y, 0);
}

static void fallout()
{

}

static void dieWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->thinkTime = 60;
		
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
		
		self->health = self->maxHealth;
		
		self->action = &doNothing;
	}
}
