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
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../projectile.h"
#include "../custom_actions.h"

extern Entity *self;

static void wait(void);
static void spit(void);
static void spitFinish(void);

Entity *addSpittingPlant(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Spitting Plant\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &wait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &entityDie;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &doNothing;

		setEntityAnimation(self, ATTACK_1);

		self->animationCallback = &spit;
	}

	checkToMap(self);
}

static void spit()
{
	Entity *e;
	int x, y, zeroDamage;

	x = self->x + self->w / 2;
	y = self->y + 5;

	zeroDamage = FALSE;

	if (strcmpignorecase(self->name, "enemy/tutorial_spitting_plant") == 0)
	{
		zeroDamage = TRUE;
	}

	e = addProjectile("common/green_blob", self, x, y, -6, 0);

	e->flags |= FLY;

	if (zeroDamage == TRUE)
	{
		e->damage = 0;
	}

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self, x, y, -6, -6);

	e->flags |= FLY;

	if (zeroDamage == TRUE)
	{
		e->damage = 0;
	}

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self, x, y, 0, -6);

	e->flags |= FLY;

	if (zeroDamage == TRUE)
	{
		e->damage = 0;
	}

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self, x, y, 6, -6);

	e->flags |= FLY;

	if (zeroDamage == TRUE)
	{
		e->damage = 0;
	}

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self, x, y, 6, 0);

	e->flags |= FLY;

	if (zeroDamage == TRUE)
	{
		e->damage = 0;
	}

	e->reactToBlock = &bounceOffShield;

	playSoundToMap("sound/common/pop.ogg", -1, self->x, self->y, 0);

	setEntityAnimation(self, ATTACK_2);

	self->animationCallback = &spitFinish;
}


static void spitFinish()
{
	setEntityAnimation(self, STAND);

	self->thinkTime = self->maxThinkTime;

	self->action = &wait;
}
