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
		takeDamage(other, other->damage);
	}

	pushEntity(other);
}

static void takeDamage(Entity *other, int damage)
{
	if (damage > 100)
	{
		self->die();
	}

	if (strcmpignorecase(self->requires, other->name) == 0)
	{
		self->health -= damage;

		setCustomAction(self, &flashWhite, 6, 0, 0);
		setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);

		if (self->health <= 0)
		{
			self->die();
		}
	}

	else if (!(self->flags & NO_DRAW))
	{
		setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);

		playSoundToMap("sound/common/dink.ogg", 2, self->x, self->y, 0);
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

	self->inUse = FALSE;

	fireTrigger(self->objectiveName);

	fireGlobalTrigger(self->objectiveName);

	playSoundToMap("sound/common/crumble.ogg", 2, self->x, self->y, 0);
}

static void fallout()
{

}
