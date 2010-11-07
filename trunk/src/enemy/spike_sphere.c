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

#include "../audio/audio.h"
#include "../graphics/decoration.h"
#include "../graphics/animation.h"
#include "../entity.h"
#include "../custom_actions.h"
#include "../system/properties.h"
#include "../system/error.h"

extern Entity *self;

static void takeDamage(Entity *, int);
static void moveVertical(void);
static void init(void);
static void moveHorizontal(void);

Entity *addSpikeSphere(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Spike Sphere");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->takeDamage = &takeDamage;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	self->action = self->maxThinkTime == 1 ? &moveHorizontal : &moveVertical;
}

static void moveVertical()
{
	self->endY += self->speed;

	if (self->endY >= 360)
	{
		self->endY = 0;
	}

	self->y = self->startY + cos(DEG_TO_RAD(self->endY)) * self->mental;
}

static void moveHorizontal()
{
	self->endX += self->speed;

	if (self->endX >= 360)
	{
		self->endX = 0;
	}

	self->x = self->startX + cos(DEG_TO_RAD(self->endX)) * self->mental;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;
	
	setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);

	playSoundToMap("sound/common/dink.ogg", 2, self->x, self->y, 0);
	
	if (other->reactToBlock != NULL)
	{
		temp = self;

		self = other;

		self->reactToBlock();

		self = temp;
	}
	
	damage = 0;
	
	addDamageScore(damage, self);
}
