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
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "enemies.h"
#include "../custom_actions.h"
#include "../hud.h"
#include "../player.h"
#include "../system/error.h"

extern Entity *self, player;
extern Game game;

static void stickToPlayer(void);
static void attack(void);
static void grab(Entity *other);
static void fallOff(void);
static void stickToPlayerAndDrain(void);

Entity *addBabySlime(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Baby Slime");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &attack;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDieNoDrop;
	e->pain = NULL;
	e->takeDamage = &entityTakeDamageFlinch;
	e->reactToBlock = NULL;
	e->touch = &grab;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void attack()
{
	long onGround = (self->flags & ON_GROUND);

	facePlayer();

	if ((self->flags & ON_GROUND) && (prand() % 30 == 0))
	{
		self->dirX = (self->face == LEFT ? -self->speed : self->speed);

		self->dirY = -(8 + prand() % 4);
	}

	checkToMap(self);

	if (onGround == 0 && ((self->flags & ON_GROUND) || (self->standingOn != NULL)))
	{
		self->dirX = 0;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->touch = NULL;

		self->die();
	}
}

static void grab(Entity *other)
{
	if (self->health <= 0)
	{
		return;
	}

	if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}
	}

	else if (other->type == PROJECTILE && other->parent != self)
	{
		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage);
		}

		other->inUse = FALSE;
	}

	else if (other->type == PLAYER && !(self->flags & GRABBING))
	{
		self->startX = (prand() % (other->w / 2)) * (prand() % 2 == 0 ? 1 : -1);

		self->startY = prand() % (other->h - self->h);

		setCustomAction(other, &slowDown, 3, 1, 0);

		if (strcmpignorecase(self->name, "enemy/red_baby_slime") == 0)
		{
			self->action = &stickToPlayerAndDrain;
		}

		else
		{
			self->action = &stickToPlayer;
		}

		self->touch = NULL;

		self->flags |= GRABBING;

		self->layer = FOREGROUND_LAYER;

		other->flags |= GRABBED;

		self->thinkTime = 0;

		self->mental = 3 + (prand() % 3);
	}
}

static void stickToPlayer()
{
	setCustomAction(&player, &slowDown, 3, 0, 0);

	if (game.showHints == TRUE)
	{
		setInfoBoxMessage(0, _("Quickly turn left and right to shake off the slimes!"));
	}

	self->x = player.x + (player.w - self->w) / 2 + self->startX;
	self->y = player.y + self->startY;

	self->thinkTime++;

	if (self->face != player.face)
	{
		self->face = player.face;

		if (self->thinkTime <= 15)
		{
			self->mental--;
		}

		self->thinkTime = 0;
	}

	if (self->mental <= 0)
	{
		self->dirX = self->speed * 2 * (prand() % 2 == 0 ? -1 : 1);

		self->dirY = -6;

		setCustomAction(&player, &slowDown, 3, -1, 0);

		self->action = &fallOff;

		player.flags &= ~GRABBED;
	}
}

static void stickToPlayerAndDrain()
{
	Entity *temp;

	setCustomAction(&player, &slowDown, 3, 0, 0);

	if (game.showHints == TRUE)
	{
		setInfoBoxMessage(0, _("Quickly turn left and right to shake off the slimes!"));
	}

	self->x = player.x + (player.w - self->w) / 2 + self->startX;
	self->y = player.y + self->startY;

	self->thinkTime++;

	if (self->face != player.face)
	{
		self->face = player.face;

		if (self->thinkTime <= 15)
		{
			self->mental--;
		}

		self->thinkTime = 0;
	}

	if (self->thinkTime >= 60)
	{
		temp = self;

		self = &player;

		self->takeDamage(temp, 1);

		self = temp;

		self->thinkTime = 0;
	}

	if (self->mental <= 0)
	{
		self->dirX = self->speed * 2 * (prand() % 2 == 0 ? -1 : 1);

		self->dirY = -6;

		setCustomAction(&player, &slowDown, 3, -1, 0);

		self->action = &fallOff;

		player.flags &= ~GRABBED;
	}
}

static void fallOff()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->die();
	}
}
