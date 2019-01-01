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
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void entityWait(void);
static void addRiftEnergy(int, int);
static void energyMoveToRift(void);

Entity *addAttractor(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Attractor");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &entityWait;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	EntityList *el, *entities;

	entities = getEntities();

	setEntityAnimation(self, self->active == FALSE ? "STAND" : "WALK");

	if (self->active == TRUE)
	{
		if (self->health == -1)
		{
			self->health = playSoundToMap("sound/item/rift", -1, self->x, self->y, -1);
		}

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			for (el=entities->next;el!=NULL;el=el->next)
			{
				if (el->entity->inUse == TRUE && el->entity->type == ENEMY &&
					collision(self->x - self->mental, self->y, self->mental * 2, self->mental * 2, el->entity->x, el->entity->y, el->entity->w, el->entity->h) == 1)
				{
					setCustomAction(el->entity, &attract, self->maxThinkTime, 0, (el->entity->x < (self->x + self->w / 2) ? self->speed : -self->speed));
				}
			}

			self->thinkTime = self->maxThinkTime;

			if (collision(self->x - self->mental, self->y, self->mental * 2, self->mental * 2, player.x, player.y, player.w, player.h) == 1)
			{
				setCustomAction(&player, &attract, self->maxThinkTime, 0, (player.x < (self->x + self->w / 2) ? self->speed : -self->speed));
			}

			stopSound(self->health);
		}

		if (prand() % 3 == 0)
		{
			addRiftEnergy(self->x + self->w / 2, self->y + self->h / 2);
		}
	}
}

static void addRiftEnergy(int x, int y)
{
	Entity *e;

	e = addBasicDecoration(x, y, "decoration/rift_energy");

	e->x += prand() % 128 * (prand() % 2 == 0 ? -1 : 1);
	e->y += prand() % 128 * (prand() % 2 == 0 ? -1 : 1);

	x -= e->w / 2;
	y -= e->h / 2;

	e->targetX = x;
	e->targetY = y;

	calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

	e->dirX *= 8;
	e->dirY *= 8;

	e->action = &energyMoveToRift;
}

static void energyMoveToRift()
{
	self->x += self->dirX;
	self->y += self->dirY;

	if (atTarget())
	{
		self->inUse = FALSE;
	}
}
