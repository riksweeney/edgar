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
#include "../audio/music.h"
#include "../collisions.h"
#include "../game.h"
#include "../decoration.h"

extern Entity *self;

static void wait(void);
static void drop(void);
static void touch(Entity *);
static void initialise(void);
static void chasePlayer(void);
static void idle(void);
static void addDust(void);

Entity *addBoulderBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add the Boulder\n");

		exit(1);
	}

	loadProperties("boss/boulder_boss", e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->die = &entityDie;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	if (self->health == 0)
	{
		if (self->active == TRUE)
		{
			self->thinkTime--;

			fadeOutMusic(3000);

			if (self->thinkTime <= 0)
			{
				self->x = self->startX;
				self->y = self->startY;

				self->touch = &touch;

				self->flags &= ~(NO_DRAW|FLY);

				self->flags |= ATTACKING;

				self->action = &drop;

				self->health = 1;
			}
		}

		else
		{
			self->thinkTime = 120;
		}
	}

	else
	{
		self->flags &= ~(NO_DRAW|FLY);

		self->touch = &pushEntity;

		self->frameSpeed = 0;

		self->action = &idle;
	}
}

static void drop()
{
	if (self->flags & ON_GROUND)
	{
		self->thinkTime = 120;

		playSoundToMap("sound/boss/boulder_boss/boulder_crash.ogg", BOSS_CHANNEL, self->x, self->y, 0);

		shakeScreen(STRONG, self->thinkTime / 2);

		addDust();

		self->action = &wait;
	}

	doNothing();
}

static void wait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		playBossMusic();

		playSoundToMap("sound/boss/boulder_boss/roll.ogg", BOSS_CHANNEL, self->x, self->y, -1);

		setEntityAnimation(self, WALK);

		self->endX = -2.0f;

		self->thinkTime = 120;

		self->action = &chasePlayer;
	}
}

static void chasePlayer()
{
	long onGround = self->flags & ON_GROUND;

	self->dirX -= 0.01f;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 120;

		self->endX -= 0.04f;

		self->frameSpeed++;
	}

	if (self->dirX <= self->endX)
	{
		self->dirX = self->endX;
	}

	checkToMap(self);

	if (onGround == 0 && (self->flags && ON_GROUND))
	{
		shakeScreen(LIGHT, 5);

		addDust();
	}

	if (self->dirX == 0)
	{
		self->action = &idle;

		self->touch = &pushEntity;

		self->frameSpeed = 0;

		self->active = FALSE;

		playSoundToMap("sound/boss/boulder_boss/boulder_crash.ogg", BOSS_CHANNEL, self->x, self->y, 0);

		shakeScreen(STRONG, 90);

		self->thinkTime = 90;

		fadeBossMusic();

		self->health = 1;

		addDust();
	}
}

static void idle()
{

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
