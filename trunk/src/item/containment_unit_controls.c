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
#include "../graphics/decoration.h"
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "key_items.h"
#include "item.h"
#include "../hud.h"
#include "../inventory.h"
#include "../custom_actions.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../collisions.h"
#include "../event/script.h"
#include "../system/error.h"
#include "../system/random.h"

extern Entity *self;

static void entityWait(void);
static void touch(Entity *);
static void init(void);
static void activate(int);
static void takeDamage(Entity *, int);

Entity *addContainmentUnitControls(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Containment Unit Controls");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;

	e->touch = &touch;

	e->activate = &activate;

	e->takeDamage = &takeDamage;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->thinkTime = 0;

	return e;
}

static void init()
{
	setEntityAnimation(self, self->active == TRUE ? STAND : WALK);

	self->takeDamage = self->active == TRUE ? takeDamage : NULL;

	self->action = &entityWait;
}

static void entityWait()
{
	checkToMap(self);
}

static void touch(Entity *other)
{
	Entity *temp;

	if (other->type == PLAYER && self->health > 0)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to retrieve Purple Gem"));
	}

	else if (other->type == WEAPON && (other->flags & ATTACKING))
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

		temp = self;

		self = other;

		self->die();

		self = temp;
	}
}

static void activate(int val)
{
	if (self->active == TRUE)
	{
		runScript(self->requires);
	}
}

static void takeDamage(Entity *other, int damage)
{
	Entity *e, *temp;

	if (strcmpignorecase(other->name, "weapon/pickaxe") == 0)
	{
		self->health--;

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);
		}

		else
		{
			self->health = 0;

			self->touch = NULL;

			e = addPermanentItem("item/purple_gem", self->x + self->w / 2, self->y);

			e->dirX = other->face == LEFT ? -6 : 6;

			e->dirY = -ITEM_JUMP_HEIGHT;

			self->active = FALSE;

			setEntityAnimation(self, WALK);

			fireTrigger(self->objectiveName);
		}
		
		addDamageScore(damage, self);
	}

	else
	{
		playSoundToMap("sound/common/dink.ogg", -1, self->x, self->y, 0);

		setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);
		
		if (other->reactToBlock != NULL)
		{
			temp = self;

			self = other;

			self->reactToBlock();

			self = temp;
		}
		
		damage = 0;
		
		addDamageScore(damage, self);
		
		if (prand() % 10 == 0)
		{
			setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
		}
	}
}
