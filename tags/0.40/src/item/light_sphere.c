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
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../graphics/graphics.h"
#include "../game.h"
#include "key_items.h"
#include "../system/error.h"

extern Entity *self;

static void wait(void);
static int draw(void);
static void takeDamage(Entity *, int);

Entity *addLightSphere(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Light Sphere");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &wait;
	e->takeDamage = &takeDamage;

	e->draw = &draw;

	setEntityAnimation(e, STAND);

	return e;
}

static int draw()
{
	drawLine(self->startX + self->w / 2, self->startY, self->x + self->w / 2, self->y, 255, 255, 255);

	drawLoopingAnimationToMap();

	return TRUE;
}

static void wait()
{
	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}
}

static void takeDamage(Entity *other, int damage)
{
	Entity *e;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (self->thinkTime <= 0)
	{
		if (other->type == PROJECTILE)
		{
			other->target = self;
		}

		setCustomAction(self, &invulnerable, 20, 0);

		self->thinkTime = 180;

		playSoundToMap("sound/enemy/gazer/flash.ogg", -1, self->x, self->y, 0);

		fadeFromWhite();

		e = getEntityByObjectiveName(self->requires);

		if (e != NULL)
		{
			e->activate(-1);
		}
	}
}
