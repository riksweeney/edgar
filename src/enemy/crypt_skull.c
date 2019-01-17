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
#include "../boss/sorceror.h"
#include "../collisions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../graphics/graphics.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self, player;

static void entityWait(void);
static void beamAttackInit(void);
static void beamAttack(void);
static void beamAttackWait(void);
static int drawBeam(void);
static void appear(void);

Entity *addCryptSkull(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Crypt Skull");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &entityWait;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;

	e->type = ENEMY;

	setEntityAnimation(e, "WALK");

	return e;
}

static void entityWait()
{
	if (self->active == TRUE)
	{
		self->action = &appear;

		self->flags &= ~NO_DRAW;

		self->alpha = 0;
	}
}

static void appear()
{
	self->alpha += 3;

	if (self->alpha >= 255)
	{
		self->alpha = 255;

		self->touch = &entityTouch;

		self->takeDamage = &entityTakeDamageNoFlinch;

		self->action = &beamAttackInit;
	}
}

static void beamAttackInit()
{
	int i, j, beams;
	Entity *e, *prev;

	prev = NULL;

	beams = self->mental == -1 ? 2 : self->mental;

	for (j=0;j<beams;j++)
	{
		for (i=0;i<16;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add a Crypt Skull Beam");
			}

			loadProperties("boss/sorceror_disintegration_spell", e);

			setEntityAnimation(e, "STAND");

			if (i == 0)
			{
				e->flags &= ~NO_DRAW;

				e->x = self->x + self->w / 2 - e->w / 2;
				e->y = self->y + self->h / 2 - e->h / 2;

				e->startX = e->x;
				e->startY = e->y;

				e->y = j == 0 ? self->startY : self->endY;

				e->draw = &drawBeam;
				e->action = &beamAttack;
				e->touch = &entityTouch;

				e->speed = self->speed;

				e->dirX = j == 0 ? e->speed : -e->speed;
				e->dirY = 0;

				e->head = self;

				self->action = &beamAttackWait;

				prev = e;

				e->damage = 1;

				if (j == 0)
				{
					e->targetX = playSoundToMap("sound/enemy/laser/zap", -1, self->x, self->y, -1);
				}
			}

			else
			{
				e->draw = &drawLoopingAnimationToMap;
				e->touch = &entityTouch;
				e->action = &doNothing;

				e->damage = 1;

				prev->target = e;

				prev = e;
			}

			e->flags |= FLY|DO_NOT_PERSIST|UNBLOCKABLE|PLAYER_TOUCH_ONLY;

			e->mental = self->mental;

			e->thinkTime = 300;

			e->health = 0;
		}
	}
}

static void beamAttack()
{
	float x, y, partDistanceX, partDistanceY, speedX, speedY;
	Entity *e;

	speedX = self->dirX;
	speedY = self->dirY;

	if (self->mental == -1)
	{
		speedX *= cos(DEG_TO_RAD(self->endX));
		speedY *= cos(DEG_TO_RAD(self->endX));

		self->endX += 0.3;
	}

	self->x += speedX;
	self->y += speedY;

	if (speedX > 0 && self->x >= self->head->endX)
	{
		self->x = self->head->endX;

		self->dirX = 0;
		self->dirY = self->speed;
	}

	else if (speedX < 0 && self->x <= self->head->startX)
	{
		self->x = self->head->startX;

		self->dirX = 0;
		self->dirY = -self->speed;
	}

	else if (speedY > 0 && self->y >= self->head->endY)
	{
		self->y = self->head->endY;

		self->dirX = -self->speed;
		self->dirY = 0;
	}

	else if (speedY < 0 && self->y <= self->head->startY)
	{
		self->y = self->head->startY;

		self->dirX = self->speed;
		self->dirY = 0;
	}

	if (self->head->health <= 0 || self->head->inUse == FALSE)
	{
		e = self->target;

		while (e != NULL)
		{
			e->inUse = FALSE;

			e = e->target;
		}

		self->inUse = FALSE;

		stopSound(self->targetX);
	}

	else
	{
		x = self->startX;
		y = self->startY;

		partDistanceX = self->x - self->startX;
		partDistanceY = self->y - self->startY;

		partDistanceX /= 16;
		partDistanceY /= 16;

		e = self->target;

		while (e != NULL)
		{
			x += partDistanceX;
			y += partDistanceY;

			e->x = (e->target == NULL ? self->startX : x);
			e->y = (e->target == NULL ? self->startY : y);

			e->damage = self->damage;

			e = e->target;
		}
	}
}

static int drawBeam()
{
	Colour colour1, colour2, colour3;
	
	colour1.r = 231;
	colour1.g = 231;
	colour1.b = 231;
	colour1.a = 255;
	
	colour2.r = 57;
	colour2.g = 57;
	colour2.b = 224;
	colour2.a = 255;
	
	colour3.r = 41;
	colour3.g = 41;
	colour3.b = 160;
	colour3.a = 255;

	drawDisintegrationLine(self->startX, self->startY, self->x, self->y, colour1, colour2, colour3);

	return TRUE;
}

static void beamAttackWait()
{

}
