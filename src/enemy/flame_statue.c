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
#include "../event/global_trigger.h"
#include "../event/trigger.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../item/item.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void die(void);
static void doFlame(void);
static void flameWait(void);
static void entityWait(void);
static void touch(Entity *);
static void flameTouch(Entity *);
static void takeDamage(Entity *, int);
static void resumeNormalFunction(void);

Entity *addFlameStatue(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Flame Statue");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &entityWait;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->touch = &touch;
	e->takeDamage = &takeDamage;
	e->resumeNormalFunction = &resumeNormalFunction;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	int i;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental = self->mental == 0 ? 1 : 0;

		self->thinkTime = 120;

		for (i=0;i<2;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add a Flame Statue Flame");
			}

			loadProperties("enemy/horizontal_flame", e);

			if (i == 0)
			{
				e->face = RIGHT;

				e->x = self->x + self->w;

				e->y = self->y + (self->mental == 1 ? self->offsetY : self->offsetX);
			}

			else
			{
				e->face = LEFT;

				e->x = self->x - e->w;

				e->y = self->y + (self->mental == 0 ? self->offsetY : self->offsetX);
			}

			e->action = &flameWait;
			e->draw = &drawLoopingAnimationToMap;
			e->touch = &flameTouch;

			e->head = self;

			e->thinkTime = self->thinkTime;

			if (i == 0)
			{
				e->endY = playSoundToMap("sound/enemy/fire_burner/flame", -1, self->x, self->y, -1);
			}

			else
			{
				e->endY = -1;
			}

			e->type = ENEMY;

			e->flags |= DO_NOT_PERSIST|PLAYER_TOUCH_ONLY;

			setEntityAnimation(e, "STAND");
		}

		self->action = &doFlame;
	}

	self->dirX = self->standingOn != NULL ? self->standingOn->dirX : 0;

	checkToMap(self);
}

static void touch(Entity *other)
{
	if (other->flags & ATTACKING)
	{
		takeDamage(other, other->damage);
	}

	if (self->inUse == TRUE && self->touch != NULL)
	{
		pushEntity(other);
	}
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		if (strcmpignorecase(other->name, "weapon/pickaxe") != 0)
		{
			playSoundToMap("sound/common/dink", -1, self->x, self->y, 0);

			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

			if (other->reactToBlock != NULL)
			{
				temp = self;

				self = other;

				self->reactToBlock(temp);

				self = temp;
			}

			if (other->type != PROJECTILE && prand() % 10 == 0)
			{
				setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
			}

			damage = 0;
		}

		else
		{
			entityTakeDamageNoFlinch(other, damage);
		}
	}
}

static void flameTouch(Entity *other)
{
	Entity *e;

	if (other->type == PLAYER && self->startX == 0)
	{
		e = addProjectile("enemy/fireball", self->head, 0, 0, (self->face == LEFT ? -8 : 8), 0);

		e->x = other->x + other->w / 2 - e->w / 2;
		e->y = other->y + other->h / 2 - e->h / 2;

		e->flags |= FLY|NO_DRAW;

		self->startX = 7;
	}
}

static void doFlame()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 30;

		self->action = &entityWait;
	}

	self->dirX = self->standingOn != NULL ? self->standingOn->dirX : 0;

	checkToMap(self);
}

static void flameWait()
{
	if (self->face == RIGHT)
	{
		self->x = self->head->x + self->head->w;

		self->y = self->head->y + (self->head->mental == 1 ? self->head->offsetY : self->head->offsetX);
	}

	else
	{
		self->x = self->head->x - self->w;

		self->y = self->head->y + (self->head->mental == 0 ? self->head->offsetY : self->head->offsetX);
	}

	self->startX--;

	if (self->startX <= 0)
	{
		self->startX = 0;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0 || self->head->health <= 0)
	{
		stopSound(self->endY);

		self->inUse = FALSE;
	}
}

static void die()
{
	int i;
	Entity *e;
	char name[MAX_VALUE_LENGTH];

	playSoundToMap("sound/common/crumble", -1, self->x, self->y, 0);

	snprintf(name, sizeof(name), "%s_piece", self->name);

	fireTrigger(self->objectiveName);

	fireGlobalTrigger(self->objectiveName);

	for (i=0;i<9;i++)
	{
		e = addTemporaryItem(name, self->x, self->y, self->face, 0, 0);

		e->x += (self->w - e->w) / 2;
		e->y += (self->w - e->w) / 2;

		e->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->thinkTime = 60 + (prand() % 180);
	}

	self->damage = 0;

	if (!(self->flags & INVULNERABLE))
	{
		self->touch = &entityTouch;

		self->flags &= ~FLY;

		self->flags |= (DO_NOT_PERSIST|NO_DRAW);

		self->thinkTime = 60;

		setCustomAction(self, &invulnerableNoFlash, 240, 0, 0);

		self->frameSpeed = 0;

		self->action = &standardDie;

		self->damage = 0;
	}
}

static void resumeNormalFunction()
{
	self->action = &entityWait;

	self->touch = &touch;
}
