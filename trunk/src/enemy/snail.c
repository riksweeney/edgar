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
#include "../custom_actions.h"

extern Entity *self, player;

static void die(void);
static void pain(void);
static void hideStart(void);
static void hide(void);
static void hideEnd(void);
static void lookForPlayer(void);
static void resume(void);
static void takeDamage(Entity *, int);

Entity *addSnail(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Snail\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->pain = &pain;
	e->takeDamage = &takeDamage;
	e->reactToBlock = &changeDirection;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void die()
{
	entityDie();
}

static void pain()
{
	playSound("sound/enemy/bat/squeak.wav", ENEMY_CHANNEL_1, ENEMY_CHANNEL_2, self->x, self->y);
}

static void lookForPlayer()
{
	checkToMap(self);

	if (self->dirX == 0 || isAtEdge(self) == TRUE)
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	if (prand() % 120 == 0)
	{
		if (collision(self->x + (self->face == RIGHT ? self->w : -320), self->y, 320, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &hideStart;
		}
	}
}

static void hideStart()
{
	self->dirX = 0;

	setEntityAnimation(self, ATTACK_1);

	self->flags |= INVULNERABLE;

	self->animationCallback = &hide;
}

static void hide()
{
	setEntityAnimation(self, ATTACK_2);

	self->action = &hide;

	if (prand() % 240 == 0)
	{
		self->frameSpeed *= -1;

		setEntityAnimation(self, ATTACK_1);

		self->action = &hideEnd;
	}
}

static void hideEnd()
{
	self->animationCallback = &resume;
}

static void resume()
{
	self->frameSpeed *= -1;

	setEntityAnimation(self, STAND);

	self->action = &lookForPlayer;
}

static void takeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		if (self->face == other->face)
		{
			printf("Dink\n");
		}

		else
		{
			self->health -= damage;

			if (self->health > 0)
			{
				setCustomAction(self, &helpless, 10, 0);
				setCustomAction(self, &invulnerable, 20, 0);
			}

			else
			{
				self->die();
			}
		}
	}
}
