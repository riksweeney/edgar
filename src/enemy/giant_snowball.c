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
#include "../collisions.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../item/item.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;
extern Input input;

static void init(void);
static void fall(void);
static void roll(void);
static void die(void);
static void shakeFree(void);
static void moveTarget(void);
static void touch(Entity *);

Entity *addGiantSnowball(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Giant Snowball");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->fallout = &die;
	e->die = &die;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case 0:
			self->frameSpeed = 0;

			self->action = &fall;
		break;

		default:
			self->action = &roll;
		break;
	}
}

static void fall()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->mental = 1;

		self->frameSpeed = 1;

		self->action = &roll;

		self->targetX = playSoundToMap("sound/boss/boulder_boss/roll", -1, self->x, self->y, -1);
	}

	moveTarget();

	syncBoulderFrameSpeed();
}

static void roll()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->health = 0;
	}

	if (self->target != NULL)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press buttons to escape the snowball!"));

		shakeFree();
	}

	if (self->health <= 0 || (self->target != NULL && self->target->health <= 0))
	{
		self->die();
	}

	else
	{
		moveTarget();
	}

	syncBoulderFrameSpeed();
}

static void shakeFree()
{
	Entity *e;

	if (player.health > 0 && (input.up == 1 || input.down == 1 || input.right == 1 || input.left == 1 ||
		input.previous == 1 || input.next == 1 || input.jump == 1 ||
		input.activate == 1 || input.attack == 1 || input.interact == 1 || input.block == 1))
	{
		self->health--;

		input.up = 0;
		input.down = 0;
		input.right = 0;
		input.left = 0;
		input.previous = 0;
		input.next = 0;
		input.jump = 0;
		input.activate = 0;
		input.attack = 0;
		input.interact = 0;
		input.block = 0;

		e = addTemporaryItem("misc/giant_snowball_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += self->w / 2 - e->w / 2;
		e->y += self->h / 2 - e->h / 2;

		e->dirX = (prand() % 6) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = -4;

		e->thinkTime = 60 + (prand() % 120);
	}
}

static void touch(Entity *other)
{
	if (self->target == NULL && other->type == PLAYER && !(other->flags & INVULNERABLE) && other->health > 0)
	{
		other->flags |= NO_DRAW;

		self->target = other;

		setPlayerLocked(TRUE);
	}
}

static void moveTarget()
{
	if (self->target != NULL)
	{
		self->target->x = self->x + self->w / 2 - self->target->w / 2;

		self->target->y = self->y + self->h / 2 - self->target->h / 2;

		setCustomAction(self->target, &invulnerableNoFlash, 2, 0, 0);
	}
}

static void die()
{
	int i;
	Entity *e;

	stopSound(self->targetX);

	self->targetX = playSoundToMap("sound/enemy/giant_snowball/crumble", -1, self->x, self->y, 0);

	for (i=0;i<32;i++)
	{
		e = addTemporaryItem("misc/giant_snowball_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += prand() % self->w;
		e->y += prand() % self->h;

		e->dirX = (prand() % 4) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = -2.5;

		e->thinkTime = 60 + (prand() % 120);
	}

	self->inUse = FALSE;

	if (self->target != NULL)
	{
		if (self->health <= 0)
		{
			self->target->dirY = ITEM_JUMP_HEIGHT;

			setCustomAction(self->target, &invulnerable, 15, 0, 0);
		}

		self->target->environment = AIR;

		self->target->flags &= ~NO_DRAW;

		setPlayerLocked(FALSE);
	}
}
